#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

const char* serverName = "http://your_server_address.com/api/v1/beers"; //change this to your server address and endpoint

const int flowPin = 34; //change this to your SF800 flow meter pin
float flowRate = 0.0;
volatile byte pulses = 0;
float calibrationFactor = 7.5; //change this to your SF800 calibration factor
Auti
void IRAM_ATTR flowPulse() {
  pulses++;
}

void setup() {
  Serial.begin(115200);
  pinMode(flowPin, INPUT_PULLUP);
  attachInterrupt(flowPin, flowPulse, FALLING);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  static unsigned long lastMeasurementTime = 0;
  unsigned long now = millis();

  if (now - lastMeasurementTime > 1000) {
    detachInterrupt(flowPin);
    flowRate = ((float)pulses / calibrationFactor) * 60.0;
    pulses = 0;
    attachInterrupt(flowPin, flowPulse, FALLING);
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.println(" L/min");

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");
      String json = "{\"flow_rate\": " + String(flowRate) + "}";
      int httpResponseCode = http.POST(json);
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    lastMeasurementTime = now;
  }
}
