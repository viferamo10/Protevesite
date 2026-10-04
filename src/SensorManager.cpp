// SensorManager.cpp — CHG-20260906-008
// Gerencia sensores do PROTEVE: 2 canais PZEM (UART1/UART2), DS18B20 (OneWire),
// RTC DS3231 (I2C) e Sensor de Porta (GPIO reed switch).
// Origem: V11.9 L290-293 (iniciacao hardware), L390-401 (aquisicao sensores).
// Pinagem canônica: hardware/PINOUT/PINOUT-S3-CANONICAL.md (R-10).
// Qualidade de dados e origem REAL/SIMULATED conforme R-06.

#include "SensorManager.h"
#include "EventEngine.h"
#include <Arduino.h>

namespace proteve {

void SensorManager::setEventEngine(EventEngine* events) {
  _events = events;
}

void SensorManager::begin(Storage& storage) {
  _storage = &storage;

  // 1) Canais PZEM (MVP: 2 canais; UART1 gpio 27/26, UART2 gpio 17/16 do PINOUT canônico)
  _pzem[0].begin(Serial1, 0x01, 27, 26);
  _pzem[1].begin(Serial2, 0x02, 17, 16);

  // 2) DS18B20 (1-Wire GPIO 4 - proposta PINOUT-S3-CANONICAL)
  _oneWire = new OneWire(4);
  _ds18b20 = new DallasTemperature(_oneWire);
  _ds18b20->begin();

  // 3) RTC DS3231 (I2C)
  _rtcPresent = _rtc.begin();
  setState("rtc_0", _rtcPresent ? SensorState::ONLINE : SensorState::OFFLINE);

  // 4) Sensor de Porta (GPIO 5 reed switch com pullup)
  pinMode(5, INPUT_PULLUP);
  _doorOpen = (digitalRead(5) == HIGH);
  setState("door_0", SensorState::ONLINE);

  // Inicializa snapshots zerados com origem REAL
  for (uint8_t i = 0; i < 2; i++) {
    _snapshots[i].ok = false;
    _snapshots[i].v = 0; _snapshots[i].a = 0; _snapshots[i].w = 0;
    _snapshots[i].pf = 0; _snapshots[i].hz = 0; _snapshots[i].kwh = 0;
    _snapshots[i].origin = DataOrigin::REAL;
    _snapshots[i].doorOpen = _doorOpen;
    _snapshots[i].tempC = -127.0f;
    _snapshots[i].ts = 0;
  }
}

void SensorManager::tick(uint32_t now) {
  // Ciclo de leitura a cada ~1500ms (V11.9 T_LEITURA_MS)
  if (now - _lastTickMs < 1500) return;
  _lastTickMs = now;

  // --- 1) Porta ---
  _doorOpen = (digitalRead(5) == HIGH);
  setState("door_0", SensorState::ONLINE);

  // --- 2) RTC DS3231 (timestamp + temperatura da placa/painel) ---
  uint32_t tsActual = now / 1000;
  if (_rtcPresent) {
    DateTime dt = _rtc.now();
    tsActual = dt.unixtime();
    _rtcTempC = _rtc.getTemperature();
    setState("rtc_0", SensorState::ONLINE);
  } else {
    setState("rtc_0", SensorState::OFFLINE);
  }

  // --- 3) DS18B20 (Temperatura da câmara) ---
  if (_ds18b20 != nullptr) {
    _ds18b20->requestTemperatures();
    float t = _ds18b20->getTempCByIndex(0);
    if (t != DEVICE_DISCONNECTED_C && t > -50.0f && t < 125.0f) {
      _lastTempC = t;
      setState("temp_chamber", SensorState::ONLINE);
    } else {
      setState("temp_chamber", SensorState::OFFLINE);
    }
  }

  // --- 4) Leituras PZEM (Canais 0 e 1) ---
  for (uint8_t i = 0; i < 2; i++) {
    PzemReading r = _pzem[i].readAll();
    const char* id = (i == 0) ? "pzem_0" : "pzem_1";

    _snapshots[i].ts = tsActual;
    _snapshots[i].v = r.v;
    _snapshots[i].a = r.i;
    _snapshots[i].w = r.p;
    _snapshots[i].kwh = r.e;
    _snapshots[i].hz = r.hz;
    _snapshots[i].pf = r.pf;
    _snapshots[i].ok = r.ok;
    _snapshots[i].doorOpen = _doorOpen;
    _snapshots[i].tempC = _lastTempC;
    _snapshots[i].origin = DataOrigin::REAL;

    if (r.ok) {
      _pzemLastReadMs[i] = now;
      setState(id, SensorState::ONLINE);
    } else {
      if (_pzemState[i] == SensorState::ONLINE && (now - _pzemLastReadMs[i] > 10000)) {
        setState(id, SensorState::STALE);
      } else if (now - _pzemLastReadMs[i] > 30000) {
        setState(id, SensorState::OFFLINE);
      }
    }
  }
}

SensorState SensorManager::stateOf(const char* sensorId) const {
  if (!sensorId) return SensorState::OFFLINE;
  if (strcmp(sensorId, "pzem_0") == 0) return _pzemState[0];
  if (strcmp(sensorId, "pzem_1") == 0) return _pzemState[1];
  if (strcmp(sensorId, "temp_chamber") == 0) return _tempState;
  if (strcmp(sensorId, "rtc_0") == 0) return _rtcState;
  if (strcmp(sensorId, "door_0") == 0) return _doorState;
  return SensorState::OFFLINE;
}

bool SensorManager::getSnapshot(uint8_t channel, Snapshot& out) const {
  if (channel >= 2) return false;
  out = _snapshots[channel];
  return _snapshots[channel].ok;
}

float SensorManager::panelTempC() const {
  // Dado operacional do painel (RECONSTRUCTION §15): temperatura do RTC ou fallback
  if (_rtcPresent) return _rtcTempC;
  if (_lastTempC > -50.0f) return _lastTempC;
  return 25.0f;
}

void SensorManager::setState(const char* sensorId, SensorState st) {
  if (!sensorId) return;

  SensorState* prev = nullptr;
  if (strcmp(sensorId, "pzem_0") == 0) prev = &_pzemState[0];
  else if (strcmp(sensorId, "pzem_1") == 0) prev = &_pzemState[1];
  else if (strcmp(sensorId, "temp_chamber") == 0) prev = &_tempState;
  else if (strcmp(sensorId, "rtc_0") == 0) prev = &_rtcState;
  else if (strcmp(sensorId, "door_0") == 0) prev = &_doorState;

  if (prev && *prev != st) {
    SensorState oldSt = *prev;
    *prev = st;

    if (_events != nullptr) {
      if (st == SensorState::ONLINE) {
        _events->emit(EventType::SENSOR_ONLINE, Severity::INFO, sensorId);
      } else if (st == SensorState::OFFLINE || st == SensorState::STALE) {
        _events->emit(EventType::SENSOR_OFFLINE, Severity::WARNING, sensorId);
      }
    }
    (void)oldSt;
  }
}

}
