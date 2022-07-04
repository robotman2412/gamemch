
#pragma once

#include "main.h"
#include "connection.h"
#include <map>

extern std::map<int, Connection*> connMap;

void bt_start();
void bt_scan();
void parseEir(uint8_t *data, size_t len);
void bluetoothSendCallback(Connection *from, const char *cstr);
