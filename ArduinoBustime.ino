// Libraries
#include <Wire.h>  // This library is already built in to the Arduino IDE
#include <LiquidCrystal_I2C.h> //This library you can add via Include Library > Manage Library > 
#include <ESP8266WiFi.h>
#include "RTClib.h"

// Init screen via i2c
LiquidCrystal_I2C lcd(0x3F, 20, 4);

// Setup vars
const char* ssid = "ENTER_SSID";// put your local wifi info here
const char* password = "ENTER_PASSWORD";// put your local wifi info here
int pin_led = 2;
int pin_led2 = 16;
int btn = 0;
int switch1 = 14;
int switch2 = 12;
int looped = 25;
int stat;

RTC_Millis rtc;
int hr_24, hr_12;
uint8_t Min;
int a,b,c,a2,b2,c2;
int accurateTime = 0;

const char* host = "bustime.mta.info";
const char* key = "INSERT_YOU_API_KEY";
const char* opr = "MTA";
const char* Stop = "300982";
const char* Stop2 = "307195";
const char* Stop3 = "300958";
int next = 2;

byte customChar[8] = {
  0b10101,
  0b00000,
  0b10001,
  0b00100,
  0b00100,
  0b10001,
  0b00000,
  0b10101
};
byte reload[8] = {
  0b00100,
  0b11110,
  0b10100,
  0b10001,
  0b10001,
  0b11011,
  0b01110,
  0b00000
};

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print("ESP8266 Started");
  
  pinMode(pin_led, OUTPUT);
  pinMode(pin_led2, OUTPUT);
  pinMode(btn, INPUT);
  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);
  pinMode(pin_led, LOW);
  pinMode(pin_led2, LOW);
  
  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight
  lcd.createChar(0, customChar);
  lcd.createChar(1, reload);
  
  rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
  delay(10);
  
 // connecting to a WiFi network
  WiFi.hostname("ESP-8266");
  WiFi.begin(ssid, password);
  lcd.setCursor(5, 1);
  lcd.print("Connecting");
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  lcd.setCursor(0, 2);
  while (WiFi.status() != WL_CONNECTED) {
    pinMode(pin_led, HIGH);
    lcd.print(" . ");
    delay(400);
    pinMode(pin_led, LOW);
    delay(400);
  }

  //Show wifi connection worked and display our ip!
  pinMode(pin_led, HIGH);
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
  delay(180000);
}

void loop() {
  if(looped > 30){
    getBus();
    looped = 0;
  }else{
    looped++;
  /*
    stat = digitalRead(switch1);
    Serial.print("Switch 1 = ");
    Serial.println(stat);
    stat = digitalRead(switch2);
    Serial.print("Switch 2 = ");
    Serial.println(stat);
    Serial.println("");
  */
    delay(1000);
  }
}

void getBus(){
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/api/siri/stop-monitoring.json";
  url += "?key=";
  url += key;
  url += "&OperatorRef=";
  url +=  opr;
  url += "&MonitoringRef=";
  if(digitalRead(btn) == 1){
    if(digitalRead(switch1) == 1){
      url += Stop;
    }else{
      url += Stop3;
    }
  }else{
    url += Stop2;
  }
  url += "&MaximumStopVisits=";
  url += next;
  //Serial.print("btn: ");
  //Serial.println(digitalRead(btn));
  //Serial.print("switch1: ");
  //Serial.println(digitalRead(switch1));

  lcd.setCursor(19, 0);
  lcd.write(byte(1));
  pinMode(pin_led2, HIGH);
  
  //Serial.println(url);
  
  // This will send the request to the server
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: bustime.mta.info");
  client.println("Connection: close\r\n");
  delay(1000);

  // Read all the lines of the reply from server and print them to Serial
  if(client.available()){
    //long line = client.read();
    //Serial.print(line);

  //int num = 0;
   //This is just a text search, not fancy.
    lcd.clear();
    pinMode(pin_led2, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Next 2 B9 Buses");
    
    if(accurateTime != 1){
      accurateTime = 1;
      client.findUntil("ResponseTimestamp\":\"" ,"");
      String rightYear = client.readStringUntil('-');
      String rightMonth = client.readStringUntil('-');
      String rightDay = client.readStringUntil('T');
      String rightHour = client.readStringUntil(':');
      String rightMinutes = client.readStringUntil(':');
      String rightSeconds = client.readStringUntil('.');
      int rYr = rightYear.toInt();
      int rMo = rightMonth.toInt();
      int rDy = rightDay.toInt();
      int rSec = rightSeconds.toInt();
      int rMin = rightMinutes.toInt();
      int rHr = rightHour.toInt();
      //rtc.adjust(DateTime(F(rightDate), F(rightHour+ ","+ rightMinutes + ","+ rightSeconds)));
      rtc.adjust(DateTime(rYr, rMo, rDy, rHr, rMin, rSec));
      //rtc.adjust(DateTime(rightDate));
    }
    Time();
    
    client.findUntil("ExpectedArrivalTime\":\"" ,"");
    String rightDate = client.readStringUntil('T');
    String hr = client.readStringUntil(':');
    String minutes = client.readStringUntil(':');
    String sec = client.readStringUntil('.');
    client.findUntil("PresentableDistance\":\"" ,"");
    String nextBusStop = client.readStringUntil(' a');
    if(nextBusStop != ""){
      a = hr.toInt();
      b = minutes.toInt();
      c = sec.toInt();
      if(a==00){
          a = 12;
      }else if(a==12){
          a = 12;
      }else{
          a = a%12;
      }
      if(b==0){
        minutes = "00";
      }else if(b<10){
        minutes = "0" + String(b);
      }else{
        minutes = String(b);
      }
      lcd.setCursor(0, 1);
      lcd.print(String(a) +":"+ String(minutes) +" , "+ nextBusStop);
    }
    
    client.findUntil("ExpectedArrivalTime\":\"" ,"");
    client.findUntil("T","");
    String hr2 = client.readStringUntil(':');
    String minutes2 = client.readStringUntil(':');
    String sec2 = client.readStringUntil('.');
    client.findUntil("PresentableDistance\":\"" ,"");
    String nbs = client.readStringUntil(' a');
    if(nbs != ""){
      a2 = hr2.toInt();
      b2 = minutes2.toInt();
      c2 = sec2.toInt();
      if(a2==00){
        a2 = 12;
      }else if(a2==12){
          a = 12;
      }else{
        a2 = a2%12;
      }
      if(b2==0){
        minutes2 = "00";
      }else if(b2<10){
        minutes2 = "0" + String(b2);
      }else{
        minutes2 = String(b2);
      }
      lcd.setCursor(0, 2);
      lcd.print(String(a2) +":"+ String(minutes2) +" , "+ nbs);
    }
  }
}

void Time(){
  DateTime now = rtc.now();
  hr_24 = now.hour();
  Min = now.minute();
  if (hr_24==00){
    hr_12=12;
  }if (hr_24==12){
    hr_12=12;
  }else{
  hr_12=hr_24%12;
  }
  lcd.setCursor(8, 3);
  lcd.print(hr_12, DEC);
  lcd.print(':');
    if (Min==0){
    lcd.print("00");
  }
  else if (Min < 10){
    lcd.print("0");
    lcd.print(now.minute(),DEC);
  }
  else{
    lcd.print(now.minute(), DEC);
  }
}

