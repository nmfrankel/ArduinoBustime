#pragma once

// ==========================================
// WIFI SETTINGS
// ==========================================
const char* WIFI_SSID     = "ENTER_SSID";
const char* WIFI_PASSWORD = "ENTER_PASSWORD";

// ==========================================
// MTA API SETTINGS
// ==========================================
const char* API_KEY       = "INSERT_YOUR_API_KEY";
const char* API_HOST      = "bustime.mta.info";
const char* API_OPERATOR  = "MTA";

// ==========================================
// BUS STOP CODES
// ==========================================
const char* STOP_MAIN     = "300982"; 
const char* STOP_ALT_1    = "307195"; 
const char* STOP_ALT_2    = "300958"; 
const int   MAX_BUS_COUNT = 2;

// ==========================================
// HARDWARE PINS
// ==========================================
const uint8_t PIN_LED_STATUS = 2;
const uint8_t PIN_LED_BUSY   = 16;
const uint8_t PIN_BTN        = 0;
const uint8_t PIN_SWITCH_1   = 14;
const uint8_t PIN_SWITCH_2   = 12;

// ==========================================
// TIMINGS
// ==========================================
const unsigned long UPDATE_INTERVAL_MS = 30000;