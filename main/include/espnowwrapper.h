
#pragma once

#include "connection.h"
#include "stdint.h"

void espnow_start();
void espnow_broadcast(const char *topic, const char *data);
void espnow_broadcast_num(const char *topic, long number);
void espnow_broadcast_float(const char *topic, float number);

void espnowRecvCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void espnowSendCallback(Connection *from, const char *cstr);
