
#include "connection.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = "connection";

// Create an empty connection.
Connection::Connection(SendCallback callback) {
	peer         = NULL;
	status       = Connection::OPENING;
	dataBuf      = new char[Connection_BUF_LEN+1];
	recvData     = false;
	sendCallback = callback;
}

// Delete the connection.
Connection::~Connection() {
	if (peer) delete peer;
	delete dataBuf;
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
	if (data == 2) {
		// Start of message.
		if (recvData) {
			ESP_LOGW(TAG, "Previous message interrupted by another.");
		}
		dataWriteIndex = 0;
		recvData = true;
		recvOverflow = false;
	} else if (data == 3) {
		// End of message.
		if (!recvData) {
			if (!recvOverflow)
				ESP_LOGW(TAG, "Message end without message start.");
			recvOverflow = false;
		} else {
			// Null-terminate data.
			recvData = false;
			dataBuf[dataWriteIndex] = 0;
			// This will edit dataBuf while decoding.
			decodeMessage(dataBuf);
		}
	} else if (dataWriteIndex >= Connection_BUF_LEN) {
		// Data buffer is full.
		ESP_LOGE(TAG, "Data buffer is full, discarding message!");
		recvData = false;
	} else {
		// There is space left.
		dataBuf[dataWriteIndex] = data;
		dataWriteIndex ++;
	}
}

// Send data to the peer without topic.
void Connection::send(const char *cstr) {
	// Pack it in a neat little buffer.
	char *buf = new char[strlen(cstr)+4];
	buf[0] = '\002';
	buf[1] = ':';
	memcpy(&buf[2], cstr, strlen(cstr));
	buf[strlen(cstr)+2] = '\003';
	buf[strlen(cstr)+3] = 0;
	// Send it along.
	sendCallback(this, buf);
	free(buf);
}

// Send data to the peer with topic.
void Connection::send(const char *topic, const char *cstr) {
	// Pack it in a neat little buffer.
	char *buf = new char[strlen(topic)+strlen(cstr)+4];
	buf[0] = '\002';
	strcpy(buf, topic);
	strcat(buf, ":");
	strcat(buf, cstr);
	strcat(buf, "\003");
	// Send it along.
	sendCallback(this, buf);
	free(buf);
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
		