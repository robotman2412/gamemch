
#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// For pin mappings.
#include "hardware.h"
// For graphics.
#include "pax_gfx.h"
// For PNG images.
#include "pax_codecs.h"
// The screen driver.
#include "ili9341.h"
// For all system settings and alike.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
// For WiFi connectivity.
#include "wifi_connect.h"
#include "wifi_connection.h"
// For exiting to the launcher.
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
// Miscellaneous.
#include "time.h"
#include "sys/time.h"

// Updates the screen with the last drawing.
void disp_flush();

// Exits the app, returning to the launcher.
void exit_to_launcher();

#ifdef __cplusplus
}
#endif //__cplusplus

// For BlueTooth connectivity.
#include "btwrapper.h"
