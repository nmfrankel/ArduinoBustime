// Libraries
#include <Arduino_JSON.h>    			// Convert Wifi res to JSON (Alt try arduinojson.org/) 
#include <ESP8266WiFi.h>				// Wifi usage on the board

// General vars
String ssid = "ENTER_SSID";				// put your local wifi info here
String password = "ENTER_PASSWORD";		// put your local wifi info here
int cycleCount = 0;

// Initialize the Wifi client library
WiFiClient client;

void setup(){
  // start serial connection
	Serial.begin(115200);
	Serial.println();
	Serial.print("[Status] ESP8266 Started\nAttempting to connect");

	// connecting to a WiFi network
	WiFi.hostname("ESP-8266");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(" . ");
		delay(750);
	}
	Serial.println("[Status] WiFi Connected\nIP address: " + WiFi.localIP());
}

void loop(){
	if(cycleCount/30 == 1){
		getBus();
		cycleCount = 0;
	}

	cycleCount++;
	delay(1000);
}

void getBus(){
  // if last network request is still going
	client.stop();

	// now creating URI for request
	String api_key = "a6219025-4524-4d8a-98da-251c22355cbd";
	String host = "bustime.mta.info";
	String stop1 = "300982";
	String stop2 = "307195";
	String stop3 = "300958";
	
	// create URI for request w/ params
	String URI = "/api/siri/stop-monitoring.json";
	String params = "?key=" + api_key + "&MonitoringRef=" + stop1 + "&MaximumStopVisits=" + String(3);

	// wifi check && connect if needed
	// wifi check && connect if needed

	// make API call
	if(!client.connect(host + URI + params, 80)){
		Serial.print("[Status] Connection failed");
		return;
	}
	
	// Check HTTP status
	char status[32] = {0};
	client.readBytesUntil('\r', status, sizeof(status));
	if (strcmp(status, "HTTP/1.1 200 OK") != 0){
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
	}

  // Skip HTTP headers
	char endOfHeaders[] = "\r\n\r\n";
	if (!client.find(endOfHeaders)){
    Serial.println(F("Invalid response"));
    return;
	}

	// process JSON
	JSONVar busInfoArray = JSON.parse(String(client));
	// JSONVar busCount = busInfoArray['Siri']['StopMonitoringDelivery'][0]['MonitoredStopVisit'].keys();
	// for (int i = 0; i < busCount.length(); i++) {

	// display next busses
	Serial.print("Here are the next 3 busses: ");
	// busInfoArray['Siri']['StopMonitoringDelivery'][0]['MonitoredStopVisit'][0]['MonitoredVehicleJourney']['PublishedLineName']
	for (int i = 0; i < busInfoArray.length(); i++) {
		Serial.print(busInfoArray['Siri']['StopMonitoringDelivery'][0]['MonitoredStopVisit'][i]['MonitoredVehicleJourney']['DestinationName']);
		Serial.print(busInfoArray['Siri']['StopMonitoringDelivery'][0]['MonitoredStopVisit'][i]['MonitoredVehicleJourney']['MonitoredCall']['ExpectedArrivalTime']);
		Serial.print(busInfoArray['Siri']['StopMonitoringDelivery'][0]['MonitoredStopVisit'][i]['MonitoredVehicleJourney']['MonitoredCall']['Extensions']['Distances']['PresentableDistance']);
		Serial.print(busInfoArray['Siri']['StopMonitoringDelivery'][0]['MonitoredStopVisit'][i]['MonitoredVehicleJourney']['MonitoredCall']['Extensions']['Distances']['StopsFromCall']);
		if(i+1 != busInfoArray.length()) Serial.print('---------------');
	}
}
