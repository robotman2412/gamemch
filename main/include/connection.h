
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <vector>

class Connection;
extern std::vector<Connection *> connections;
extern Connection *firstNearby;

#include "player.h"

// Timeout after last message to consider connection closed.
#define Connection_TIMEOUT       5000
// Maximum chunk size.
#define Connection_CHUNK_LEN     (250 - 4) // 246
// Maximum text data size.
#define Connection_BUF_LEN       (Connection_CHUNK_LEN - 3 - 8) // 235
// Initialisation vector for checksum.
#define Connection_CHECKSUM_INIT 0xdcba5206

// Start the connection upkeeper task.
void connection_start();

// An estimate of nearby people.
extern int nearby;

class Connection {
	public:
		// Data handling state.
		typedef enum {
			// Not awaiting any data.
			IDLE,
			// Receiving part index.
			PARTNUM,
			// Receiving part total.
			TOTALNUM,
			// Receiving data.
			DATA,
			// Receiving checksum of data.
			CHECKSUM,
			// Errored transmission.
			BAD_TRANS,
		} RecvPhase;
		// Status of a connection.
		typedef enum {
			// Connection is being opened.
			OPENING,
			// Connection is open.
			OPEN,
			// Connection is being closed.
			CLOSING,
			// Connection is closed normally.
			CLOSED,
			// Connection is closed due to error.
			ERROR,
		} Status;
	
	private:
		// Current receive state.
		RecvPhase recvPhase;
		// Raw data buffer before decoding parts.
		char     *dataBuf;
		// Write index in data buffer.
		size_t    dataWriteIndex;
		// Checksum of data.
		uint32_t  dataChecksum;
		// Checksum received.
		uint32_t  realChecksum;
		// Part number received.
		uint8_t   partNum;
		// Total part numbers to receive.
		uint8_t   partTotal;
		// Expected number left to receive for partnum or checksum.
		size_t    remaining;
		// Handle and decode message data.
		void decodeMessage(char *data);
		
	public:
		// Callback for data recieved events.
		typedef void(*DataCallback)(Connection *from, const char *type, const char *data);
		// Callback for status events.
		typedef void(*StatusCallback)(Connection *from);
		// Callback to send data to peer.
		typedef void(*SendCallback)(Connection *from, const char *data);
		
		// Player associated with this connection.
		Player *player;
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
		// Number of times to retransmit messages that aren't confirmed.
		size_t retransmit;
		// Time after which to consider the connection closed.
		uint64_t timeout;
		
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
		void send(const char *topic, const char *cstr);
		// Send integer to the peer.
		void sendNum(const char *topic, long data);
		// Send float data to the peer.
		void sendFloat(const char *topic, float data);
		
		// Sets the status and notifies all listeners.
		void setStatus(Status newStatus);
		// Get the name of the status thingy.
		const char *statusToName();
};
