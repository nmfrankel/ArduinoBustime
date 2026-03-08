#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "RTClib.h"
#include "config.h"

LiquidCrystal_I2C lcd(0x3F, 20, 4);
WiFiClient client;
HTTPClient http;
RTC_Millis rtc;
unsigned long lastUpdateMillis = 0;
bool isTimeSynced = false;

byte customCharWifi[8] = {0b10101, 0b00000, 0b10001, 0b00100, 0b00100, 0b10001, 0b00000, 0b10101};
byte customCharReload[8] = {0b00100, 0b11110, 0b10100, 0b10001, 0b10001, 0b11011, 0b01110, 0b00000};

void setup() {
	Serial.begin(115200);
	Serial.println("\n\nESP8266 Started");

	initHardware();
	initLCD();
	connectToWiFi();
}

void loop() {
	maintainWiFi();
	updateClock();

	unsigned long currentMillis = millis();
	if (currentMillis - lastUpdateMillis >= UPDATE_INTERVAL_MS || lastUpdateMillis == 0) {
		lastUpdateMillis = currentMillis;
		fetchAndDisplayBusData();
	}
}

// ==========================================
// INITIALIZATION FUNCTIONS
// ==========================================

void initHardware() {
	pinMode(PIN_LED_STATUS, OUTPUT);
	pinMode(PIN_LED_BUSY, OUTPUT);
	pinMode(PIN_BTN, INPUT);
	pinMode(PIN_SWITCH_1, INPUT_PULLUP);
	pinMode(PIN_SWITCH_2, INPUT_PULLUP);

	digitalWrite(PIN_LED_STATUS, LOW);
	digitalWrite(PIN_LED_BUSY, LOW);

	// Initial rough time
	rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
}

void initLCD() {
	lcd.init();
	lcd.backlight();
	lcd.createChar(0, customCharWifi);
	lcd.createChar(1, customCharReload);
}

void connectToWiFi() {
	WiFi.hostname("ESP8266");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	lcd.setCursor(5, 1);
	lcd.print("Connecting");

	while (WiFi.status() != WL_CONNECTED) {
		digitalWrite(PIN_LED_STATUS, HIGH);
		lcd.print(".");
		delay(400);
		digitalWrite(PIN_LED_STATUS, LOW);
		delay(400);
	}

	digitalWrite(PIN_LED_STATUS, HIGH);
	lcd.clear();
	lcd.setCursor(3, 0);
	lcd.print("WiFi Connected");
	lcd.setCursor(3, 1);
	lcd.print("IP address: ");
	lcd.setCursor(3, 2);
	lcd.print(WiFi.localIP());
	lcd.setCursor(0, 3);
	lcd.write(byte(0));
	lcd.setCursor(19, 3);
	lcd.write(byte(0));

	delay(3000);
	lcd.clear();
}

void maintainWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost. Reconnecting...");
        lcd.setCursor(19, 3);
        lcd.print("!");
        
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        int retry = 0;
        while (WiFi.status() != WL_CONNECTED && retry < 10) {
            delay(500);
            retry++;
        }
    }
}

// ==========================================
// CORE APPLICATION LOGIC
// ==========================================

void fetchAndDisplayBusData() {
	String selectedStop = STOP_ALT_2;
	if (digitalRead(PIN_BTN) == HIGH) {
		selectedStop = (digitalRead(PIN_SWITCH_1) == HIGH) ? STOP_MAIN : STOP_ALT_2;
	}

	const String fullUrl = "http://" + String(API_HOST) + "/api/siri/stop-monitoring.json" +
						   "?key=" + API_KEY +
						   "&OperatorRef=" + API_OPERATOR +
						   "&MonitoringRef=" + selectedStop +
						   "&MaximumStopVisits=" + MAX_BUS_COUNT;

	lcd.setCursor(19, 0);
	lcd.write(byte(1));
	digitalWrite(PIN_LED_BUSY, HIGH);

	http.setReuse(true);
	if (http.begin(client, fullUrl)) {
		int httpCode = http.GET();

		if (httpCode == HTTP_CODE_OK) {
			WiFiClient &stream = http.getStream();
			parseBusDataWithJSON(stream);
		} else {
			Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}
		http.end();
	} else {
		Serial.println("[HTTP] Unable to connect");
	}
}

void parseBusDataWithJSON(WiFiClient &client) {
	JsonDocument filter;
	filter["Siri"]["ServiceDelivery"]["ResponseTimestamp"] = true;
	JsonObject visitFilter = filter["Siri"]["ServiceDelivery"]["StopMonitoringDelivery"][0]["MonitoredStopVisit"][0];
	visitFilter["MonitoredVehicleJourney"]["DestinationName"] = true;
	visitFilter["MonitoredVehicleJourney"]["MonitoredCall"]["ExpectedArrivalTime"] = true;
	visitFilter["MonitoredVehicleJourney"]["MonitoredCall"]["Extensions"]["Distances"]["PresentableDistance"] = true;
	visitFilter["MonitoredVehicleJourney"]["MonitoredCall"]["Extensions"]["StopsFromCall"] = true;

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

	if (error) {
		Serial.print("JSON Parsing failed: ");
		Serial.println(error.c_str());
		return;
	}

	const char* ts = doc["Siri"]["ServiceDelivery"]["ResponseTimestamp"].as<const char*>();
    if (ts != nullptr) {
        int year, month, day, hr, mn, sec;
        if (sscanf(ts, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hr, &mn, &sec) == 6) {
            rtc.adjust(DateTime(year, month, day, hr, mn, sec));
            isTimeSynced = true;
        }
    }

	JsonArray buses = doc["Siri"]["ServiceDelivery"]["StopMonitoringDelivery"][0]["MonitoredStopVisit"];

	lcd.clear();
	if (buses.isNull() || buses.size() == 0) {
		lcd.setCursor(0, 0);
		lcd.print("No buses found");
		return;
	}

	for (int row = 0; row < MAX_BUS_COUNT && row < buses.size(); row++) {
		JsonObject bus = buses[row];
		const char* arrivalTimeRaw = bus["MonitoredVehicleJourney"]["MonitoredCall"]["ExpectedArrivalTime"];
		const char* distance = bus["MonitoredVehicleJourney"]["MonitoredCall"]["Extensions"]["Distances"]["PresentableDistance"];

		if (!arrivalTimeRaw) {
			continue;
		}

		int hr, mn, sec;
		// We only need hr and mn, so we use %*d to skip the year/month/day/sec
		// Format: 2026-03-08T14:05:00Z
		if (sscanf(arrivalTimeRaw, "%*d-%*d-%*dT%d:%d:%d", &hr, &mn, &sec) == 3) {
			int displayHr = (hr % 12 == 0) ? 12 : (hr % 12);
			char buffer[21];

			snprintf(buffer, sizeof(buffer), "%d:%02d, %s",
					displayHr, mn,
					distance ? distance : "N/A");

			lcd.setCursor(0, row + 1);
			lcd.print(buffer);
		}
	}
}

void updateClock() {
  DateTime now = rtc.now();
  int displayHr = (now.hour() % 12 == 0) ? 12 : (now.hour() % 12);

  char timeBuffer[9];
  snprintf(timeBuffer, sizeof(timeBuffer), "%2d:%02d", displayHr, now.minute());

  lcd.setCursor(8, 3);
  lcd.print(timeBuffer);
}