#pragma once
#include "Types.h"
#include "Storage.h"
#include "PzemDriver.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RTClib.h>

namespace proteve {
// Gerencia todos os sensores (PZEMx3, DS18B20, porta, DS3231) e seus estados.
// Emite SENSOR_ONLINE/SENSOR_OFFLINE. Marca STALE por timeout e INVALID por leitura absurda.
class EventEngine; // fwd

class SensorManager {
public:
  void begin(Storage& storage);
  void tick(uint32_t now);
  SensorState stateOf(const char* sensorId) const;
  bool  getSnapshot(uint8_t channel, Snapshot& out) const; // canal = medidor
  float panelTempC() const;  // temperatura do painel — dado operacional (RECONSTRUCTION §15)
  void  setEventEngine(EventEngine* events);

private:
  void setState(const char* sensorId, SensorState st);

  Storage* _storage = nullptr;
  EventEngine* _events = nullptr;

  PzemDriver _pzem[2];
  Snapshot   _snapshots[2];
  SensorState _pzemState[2] = { SensorState::OFFLINE, SensorState::OFFLINE };

  OneWire* _oneWire = nullptr;
  DallasTemperature* _ds18b20 = nullptr;
  float _lastTempC = -127.0f;
  SensorState _tempState = SensorState::OFFLINE;

  RTC_DS3231 _rtc;
  bool _rtcPresent = false;
  float _rtcTempC = 25.0f;
  SensorState _rtcState = SensorState::OFFLINE;

  bool _doorOpen = false;
  SensorState _doorState = SensorState::ONLINE;

  uint32_t _lastTickMs = 0;
  uint32_t _pzemLastReadMs[2] = {0, 0};
};
}
