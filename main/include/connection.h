
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <vector>

#pragma once

#define Connection_BUF_LEN 2048

class Connection {
	private:
		// Whether data is being awaited.
		bool   recvData;
		// Whether the overflow warning has been posted.
		bool   recvOverflow;
		// Raw data buffer.
		char  *dataBuf;
		// Write index in data buffer.
		size_t dataWriteIndex;
		
	public:
		// Status of a connection.
		typedef enum {
			OPENING,
			OPEN,
			CLOSING,
			CLOSED,
			ERROR,
		} Status;
		// Callback for data recieved events.
		typedef void(*DataCallback)(Connection *from, const char *cstr);
		// Callback for status events.
		typedef void(*StatusCallback)(Connection *from);
		// Callback to send data to peer.
		typedef void(*SendCallback)(Connection *from, const char *cstr);
		
		// Current connection status.
		Status status;
		// List of data callbacks for when a message arrives.
		std::vector<DataCallback> dataCallbacks;
		// List of status callbacks for when the status changes.
		std::vector<StatusCallback> statusCallbacks;
		// Name of the peer.
		char *peer;
		// Send callback of the connection.
		SendCallback sendCallback;
		
		// Create a connection with a send callback.
		Connection(SendCallback callback);
		// Delete the connection.
		~Connection();
		
		// Called by the connection's task when multiply bytes of data are recieved.
		void onData(const uint8_t *data, size_t length);
		// Called by the connection's task when multiply bytes of data are recieved.
		void onData(const char *data, size_t length);
		// Called by the connection's task when one byte of data is recieved.
		void onData(uint8_t data);
		// Called by the connection's task when one byte of data is recieved.
		void onData(char data);
		
		// Send data to the peer.
		void send(const char *cstr);
		
		// Sets the status and notifies all listeners.
		void setStatus(Status newStatus);
		
		// Get the name of the status thingy.
		const char *statusToName();
};
