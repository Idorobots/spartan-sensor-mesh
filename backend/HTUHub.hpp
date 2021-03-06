#ifndef __HTUHUB_HPP__
#define __HTUHUB_HPP__

#include <Wire.h>
#include <SparkFunHTU21D.h>
#include "Sensor.hpp"
#include "SensorHub.hpp"
#include "Device.hpp"
#include "Reading.hpp"

#define SAMPLE_BACKLOG 30

class HTUHub: public SensorHub {
private:
  uint8_t address;
  uint8_t sda;
  uint8_t scl;
  HTU21D sensor;
  Sensor humidity;
  Sensor temperature;

public:
  HTUHub(uint8_t sda, uint8_t scl, uint8_t addr);
  void begin();
  void update();
  void connect(Device *d) const;
};

#endif
