
#include "connection.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "string.h"
#include <vector>

static TaskHandle_t handle;
std::vector<Connection *> connections;

static const char *TAG = "connection";
int nearby = 0;

static inline int8_t unhexc(char data) {
	switch (data) {
		case '0' ... '9':
			return data - '0';
		case 'a' ... 'f':
			return data - 'a' + 0xa;
		case 'A' ... 'F':
			return data - 'A' + 0xa;
		default:
			return -1;
	}
}

// Process a byte into the checksum.
static uint32_t checksumIngest(uint32_t prevState, uint8_t data) {
	// Roll left 3 bits.
	prevState  = (prevState << 3) | (prevState >> 29);
	// Simple permute.
	prevState ^= prevState << 7;
	// Add new data to it.
	prevState ^= (uint8_t) (data + 0x15);
	
	return prevState;
}



// Create an empty connection.
Connection::Connection(SendCallback callback) {
	peer         = NULL;
	status       = Connection::OPENING;
	dataBuf      = new char[Connection_BUF_LEN+1];
	recvPhase    = Connection::IDLE;
	sendCallback = callback;
	retransmit   = 3;
	dataCallbacks.push_back(playerDataCallback);
	statusCallbacks.push_back(playerStatusCallback);
	player       = new Player();
	// Add this to the connection list.
	connections.push_back(this);
}

// Delete the connection.
Connection::~Connection() {
	if (peer) delete peer;
	delete dataBuf;
	delete player;
}



// Handle and decode message data.
void Connection::decodeMessage(char *data) {
	char *msg;
	while (data && *data) {
		// Find length of part.
		char *end = strchr(data, ';');
		size_t len;
		if (end) {
			len = end - data;
			*end = 0;
		} else {
			len = strlen(data);
		}
		
		// Find splitter.
		char *split = strchr(data, ':');
		if (split) {
			*split = 0;
			msg = split + 1;
			// Notify listeners.
			for (size_t i = 0; i < dataCallbacks.size(); i++) {
				dataCallbacks[i](this, data, msg);
			}
		} else {
			// No splitter, no topic.
			for (size_t i = 0; i < dataCallbacks.size(); i++) {
				dataCallbacks[i](this, "", data);
			}
		}
		
		
		// ASet pointer to next part, if any.
		data = end ? end + 1 : 0;
	}
}

// Called by the connection's task when multiply bytes of data are recieved.
void Connection::onData(const uint8_t *data, size_t length) {
	this->onData((const char *) data, length);
}

// Called by the connection's task when multiply bytes of data are recieved.
void Connection::onData(const char *data, size_t length) {
	for (size_t i = 0; i < length; i++) {
		onData(data[i]);
	}
}

// Called by the connection's task when one byte of data is recieved.
void Connection::onData(uint8_t data) {
	onData((char) data);
}

// Called by the connection's task when one byte of data is recieved.
void Connection::onData(char data) {
	// To check whether or not to ingest the data.
	RecvPhase prevPhase = recvPhase;
	
	if (data == 3) {
		// Start of message data (ascii).
		if (recvPhase != Connection::IDLE && recvPhase != Connection::BAD_TRANS) {
			ESP_LOGW(TAG, "Previous message interrupted by another.");
		}
		dataWriteIndex = 0;
		dataChecksum   = Connection_CHECKSUM_INIT;
		recvPhase      = Connection::DATA;
		
	} else if (data == 4) {
		// Start of checksum (hex data).
		if (recvPhase != Connection::DATA) {
			if (recvPhase != Connection::BAD_TRANS)
				ESP_LOGW(TAG, "Incorrect data format (%d -> 5).", (int) recvPhase);
			recvPhase = Connection::BAD_TRANS;
			return;
		}
		realChecksum   = 0;
		recvPhase      = Connection::CHECKSUM;
		
	} else if (data == 5) {
		// End of message.
		if (recvPhase != Connection::CHECKSUM) {
			if (recvPhase != Connection::BAD_TRANS)
				ESP_LOGW(TAG, "Incorrect data format (%d -> 5).", (int) recvPhase);
		} else {
			// Verify checksum.
			if (realChecksum != dataChecksum) {
				ESP_LOGW(TAG, "Incorrect checksum (%08x should be %08x)", dataChecksum, realChecksum);
			}
			// Null-terminate data.
			dataBuf[dataWriteIndex] = 0;
			// Touch timeout.
			timeout = esp_timer_get_time() / 1000 + Connection_TIMEOUT;
			if (status != Connection::OPEN) setStatus(Connection::OPEN);
			// This will edit dataBuf while decoding.
			decodeMessage(dataBuf);
		}
		recvPhase = Connection::IDLE;
		
	}
	
	// Receiving data?
	if (recvPhase != Connection::CHECKSUM && recvPhase != Connection::IDLE) {
		// If so, process checksum.
		dataChecksum = checksumIngest(dataChecksum, data);
	}
	
	// If we didn't just change to next phase...
	if (recvPhase == prevPhase) {
		int8_t hexc = unhexc(data);
		switch (recvPhase) {
			case(PARTNUM):
				if (hexc == -1) goto bad_hex;
				partNum = (partNum << 4) | (uint8_t) hexc;
				break;
				
			case(TOTALNUM):
				if (hexc == -1) goto bad_hex;
				partTotal = (partTotal << 4) | (uint8_t) hexc;
				break;
				
			case(DATA):
				if (dataWriteIndex == Connection_CHUNK_LEN) {
					// Too much.
					ESP_LOGW(TAG, "Too much data for buffer");
				} else {
					// Still fits.
					dataBuf[dataWriteIndex++] = data;
				}
				break;
				
			case(CHECKSUM):
				if (hexc == -1) goto bad_hex;
				realChecksum = (realChecksum << 4) | (uint8_t) hexc;
				break;
				
			default:
				break;
				
			bad_hex:
				ESP_LOGW(TAG, "Bad hexadecimal '%c' in phase %d", data, recvPhase);
				recvPhase = Connection::BAD_TRANS;
				break;
		}
	}
}



// Send data to the peer with topic.
void Connection::send(const char *topic, const char *cstr) {
	// Size check.
	size_t buf_weight = strlen(topic) + strlen(cstr) + 1;
	if (buf_weight >= Connection_BUF_LEN) {
		ESP_LOGE(TAG, "Too much data to send! (%d of %d)", buf_weight, Connection_BUF_LEN);
		return;
	}
	
	// Pack it in a neat little buffer.
	char *buf = new char[Connection_CHUNK_LEN+1];
	snprintf(buf, Connection_CHUNK_LEN+1, "\003%s:%s", topic, cstr);
	// Calculate checksum of initial part.
	uint32_t sum = Connection_CHECKSUM_INIT;
	size_t len =  strlen(buf);
	for (size_t i = 0; i < len; i++) {
		sum = checksumIngest(sum, (uint8_t) buf[i]);
	}
	// Finalise message.
	size_t offs = strlen(buf);
	snprintf(buf+offs, Connection_CHUNK_LEN+1-offs, "\004%08x\005", sum);
	
	// Send it along.
	if (sendCallback) sendCallback(this, buf);
	delete buf;
}

// Sets the status and notifies all listeners.
void Connection::setStatus(Status newStatus) {
	status = newStatus;
	// Notify listeners.
	for (size_t i = 0; i < statusCallbacks.size(); i++) {
		statusCallbacks[i](this);
	}
}

// Get the name of the status thingy.
const char *Connection::statusToName() {
	const char *arr[] = {
		"OPENING",
		"OPEN",
		"CLOSING",
		"CLOSED",
		"ERROR",
	};
	if (status >= sizeof(arr)/sizeof(const char*)) {
		return "?";
	} else {
		return arr[status];
	}
}



// Task which keeps track of all connections.
static void connectionTask(void *ignored) {
	while (1) {
		// Wait a bit.
		vTaskDelay(pdMS_TO_TICKS(500));
		uint64_t now = esp_timer_get_time() / 1000;
		
		int new_nearby = 0;
		// Check connections for DEATH.
		for (size_t i = 0; i < connections.size(); i++) {
			Connection *conn = connections[i];
			if (!conn) continue;
			
			if (conn->timeout < now) {
				if (conn->status != Connection::CLOSED) {
					conn->setStatus(Connection::CLOSED);
					conn->timeout = now + Connection_TIMEOUT;
				} else {
					// TODO: Free it.
				}
			} else if (conn->status == Connection::OPEN) {
				new_nearby ++;
			}
		}
		nearby = new_nearby;
	}
}

// Start the connection upkeeper task.
void connection_start() {
	xTaskCreate(connectionTask, "conn worker", 2048, NULL, tskIDLE_PRIORITY, &handle);
}
