
#include "connection.h"
#include "esp_log.h"

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
			// Notify listeners.
			for (size_t i = 0; i < dataCallbacks.size(); i++) {
				dataCallbacks[i](this, dataBuf);
			}
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

// Send data to the peer.
void Connection::send(const char *cstr) {
	sendCallback(this, cstr);
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
		