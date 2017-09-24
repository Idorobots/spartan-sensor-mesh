#ifndef __APISERVER_HPP__
#define __APISERVER_HPP__

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "Sensor.hpp"
#include "index.html.gz.h"

const int WIFI_CONNECTION_TIMEOUT = 20000; // 20 seconds

class APIServer: public ESP8266WebServer {
private:
  const Sensor *sensor;
  const FS files;

  void handleApiConfig();
  void handleApiSensor();
  void handleWildcard();

public:
  APIServer(uint16_t port, const Sensor *s, const FS &fs);
  void begin();
};

#endif
