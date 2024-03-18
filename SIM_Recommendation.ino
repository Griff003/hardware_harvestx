#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>

#define wifiStatusLed D3
#define onStatusLed D4

ESP8266WiFiMulti WiFiMulti;

#define SIM800_RX D5
#define SIM800_TX D6

SoftwareSerial sim800(SIM800_TX, SIM800_RX);

const char* DEVICE_ID = "53d1d1e4-9e97-45a4-823d-17e5248166a9";
String serverName = "https://growfertile.science/recommendations";

void setup() {
  Serial.begin(9600);
  pinMode(wifiStatusLed, OUTPUT);
  pinMode(onStatusLed, OUTPUT);
  digitalWrite(onStatusLed, HIGH);

  sim800.begin(9600);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Novatech", "Nova@Tech");
  WiFiMulti.addAP("growfertile", "grow@fertile");

  if (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println(" Wifi not Connected  ");
    digitalWrite(wifiStatusLed, LOW);
  } else {
    digitalWrite(wifiStatusLed, HIGH);
    Serial.println(" Wifi Connected  ");
  }
}

String getRecommendations() {
  WiFiClient client;
  if (!client.connect(serverName, 443)) {
    return "Connection failed";
  }

  client.print(String("GET ") + serverName + "?device=" + DEVICE_ID + " HTTP/1.1\r\n" +
               "Host: " + serverName + "\r\n" +
               "Connection: close\r\n\r\n");

  String response = "";
  while (client.connected()) {
    response += client.readStringUntil('\r');
  }

  client.stop();
  return response;
}

void send_SMS(String message) {
  sim800.println("AT+CMGF=1");
  delay(1000);
  sim800.println("AT+CMGS=\"+1234567890\""); // Replace with your recipient's phone number
  delay(1000);
  sim800.print(message);
  sim800.println("\x1A");
  delay(1000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    String recommendations = getRecommendations();
    send_SMS(recommendations);
    digitalWrite(wifiStatusLed, HIGH);
    delay(2000);
  } else {
    if (WiFiMulti.run() != WL_CONNECTED) {
      digitalWrite(wifiStatusLed, LOW);
      Serial.println(" Wifi not Connected  ");
      delay(500);
      digitalWrite(wifiStatusLed, LOW);
      delay(500);
    }
  }
}
