#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include "Metric.h"
#include "index.html.h"
#include "main.css.h"
#include "main.js.h"

const int AP_TIMEOUT = 300000; // 5 minutes
const int CONNECTION_TIMEOUT = 20000; // 20 seconds
const int SAMPLE_INTERVAL = 2000; // 2 seconds
const int SAMPLE_BACKLOG = 30; // Averaged over 1 minute.
const int SENSOR = 2;
const int LED = 13;

boolean apEnabled = false;
const char* apSsid = "ClimateSensor";
const char* apPassword = "cl1m4t3p455w0r0";

boolean error = true;
int errors = 0;

Metric<float, SAMPLE_BACKLOG> humidity(0.0f, 100.0f);
Metric<float, SAMPLE_BACKLOG> temperature(-40.0f, 125.0f);

ESP8266WebServer server(80);
DHT dht(SENSOR, DHT11);

bool connect(char *ssid, char *password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait for connection
  uint32_t i = 0;
  while((WiFi.status() != WL_CONNECTED) && (i < CONNECTION_TIMEOUT)) {
    Serial.print(".");
    delay(500);
    i += 500;
  }

  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection failed.");
    return false;
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

void handleIndex() {
  digitalWrite(LED, 1);
  Serial.println("Serving /");
  server.send_P(200, PSTR("text/html"), index_html);
  digitalWrite(LED, 0);
}

void handleJS() {
  digitalWrite(LED, 1);
  Serial.println("Serving /main.js");
  server.send_P(200, PSTR("application/javascript"), main_js);
  digitalWrite(LED, 0);
}

void handleCSS() {
  digitalWrite(LED, 1);
  Serial.println("Serving /main.css");
  server.send_P(200, PSTR("text/css"), main_css);
  digitalWrite(LED, 0);
}

void handleConfig() {
  digitalWrite(LED, 1);
  Serial.println("Serving /config");

  char ssid[128] = "";
  char pass[128] = "";

  for(uint8_t i = 0; i < server.args(); ++i) {
    String name = server.argName(i);
    String arg = server.arg(i);
    if (name == "ssid") {
      arg.toCharArray(ssid, 128);
    } else if (name == "pass") {
      arg.toCharArray(pass, 128);
    }
  }

  if(connect(ssid, pass)) {
    server.send(200, "text/plain", "OK!");
  } else {
    server.send(401, "text/plain", "Not authorized!");
  }
  digitalWrite(LED, 0);
}

void handleSensor() {
  digitalWrite(LED, 1);
  Serial.println("Serving /sensor");

  String message = "{";

  message += "\"humidity\":{";
  message += "\"val\":";
  message += String(humidity.value(), 2);
  message += ",\"avg\":";
  message += String(humidity.average(), 2);
  message += ",\"var\":";
  message += String(humidity.variance(), 2);
  message += ",\"min\":";
  message += String(humidity.minimum(), 2);
  message += ",\"max\":";
  message += String(humidity.maximum(), 2);
  message += "}";

  message += ",\"temperature\":{";
  message += "\"val\":";
  message += String(temperature.value(), 2);
  message += ",\"avg\":";
  message += String(temperature.average(), 2);
  message += ",\"var\":";
  message += String(temperature.variance(), 2);
  message += ",\"min\":";
  message += String(temperature.minimum(), 2);
  message += ",\"max\":";
  message += String(temperature.maximum(), 2);
  message += "}";

  message +=",\"errors\":";
  message += String(errors);
  message += ",\"measurements\":";
  message += String(temperature.count());
  message += ",\"status\":";
  message += error ? "\"error\"" : "\"ok\"";

  message += "}";

  server.send(200, "application/json", message);
  digitalWrite(LED, 0);
}

void handleNotFound() {
  digitalWrite(LED, 1);
  Serial.println("Serving 404");
  server.send(404, "text/plain", "File Not Found");
  digitalWrite(LED, 0);
}

void setup(void){
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);

  Serial.begin(115200);
  Serial.println("");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSsid, apPassword);
  apEnabled = true;

  Serial.print("Access point on: ");
  Serial.println(apSsid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleIndex);
  server.on("/main.js", handleJS);
  server.on("/main.css", handleCSS);
  server.on("/config", handleConfig);
  server.on("/sensor", handleSensor);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  dht.begin();
  readSensor(0);
  Serial.println("Sensor initialized");
}

void readSensor(uint32_t currTime) {
  static uint32_t lastSampleTime = -SAMPLE_INTERVAL;

  if((currTime - lastSampleTime) < SAMPLE_INTERVAL) {
    return;
  }

  float hum = dht.readHumidity(true);
  float temp = dht.readTemperature(false, true);
  lastSampleTime = currTime;

  if(!isnan(hum) && !isnan(temp)) {
    humidity.add(hum);
    temperature.add(temp);
    error = false;
  } else {
    errors++;
    error = true;
  }
}

void timeoutAP(uint32_t currTime) {
  if(apEnabled && currTime > AP_TIMEOUT) {
    Serial.println("Disabling access point.");
    WiFi.mode(WIFI_STA);
    apEnabled = false;
  }
}

void loop(void){
  uint32_t currTime = millis();
  readSensor(currTime);
  timeoutAP(currTime);
  server.handleClient();
}