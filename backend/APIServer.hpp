#ifndef __APISERVER_HPP__
#define __APISERVER_HPP__

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "Device.hpp"

const int WIFI_CONNECTION_TIMEOUT = 20000; // 20 seconds

class APIServer: public ESP8266WebServer {
private:
  const Device *device = nullptr;
  FS &files;

  void handleOptions();
  void handleApiLogin();
  void handleApiConfig();
  void handleApiData();
  void handleWildcard();

public:
  APIServer(uint16_t port, const Device *d, FS &fs);
  void begin();
};

#endif
