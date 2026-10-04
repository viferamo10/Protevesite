// EventEngine.cpp — CHG-20260906-010
// P0.3 do ROADMAP: TUDO vira evento, append-only (R-08).
// Edge-first: ring buffer RAM (32) + persistência JSONL no Storage (SD → LittleFS fallback).
// Origem conceitual: log de eventos do V11.9 (SPIFFS) — aqui reestruturado como engine.

#include "EventEngine.h"
#include "Storage.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {

void EventEngine::begin(Storage& storage) {
  _storage = &storage;
  _eventCount = 0;
  _ringHead = 0;
  emit(EventType::BOOT, Severity::INFO, "EVENTENGINE", nullptr, "{\"note\":\"event engine online\"}");
}

void EventEngine::emit(EventType type, Severity sev, const char* source,
                       const char* assetId, const char* payloadJson) {
  EventRecord e;
  e.ts = millis();  // TODO(P0.0-bancada): preferir epoch do RTC (via TelemetryEngine/SensorManager)
  e.type = type;
  e.sev = sev;
  snprintf(e.source, sizeof(e.source), "%s", source ? source : "-");
  snprintf(e.assetId, sizeof(e.assetId), "%s", assetId ? assetId : "-");
  snprintf(e.payloadJson, sizeof(e.payloadJson), "%s", payloadJson ? payloadJson : "");

  // Ring buffer (consulta local sem SD)
  _ring[_ringHead] = e;
  _ringHead = (_ringHead + 1) % RING_SIZE;
  _eventCount++;

  // Persistência JSONL (fonte da verdade operacional; Storage é não-fatal)
  char line[192];
  snprintf(line, sizeof(line),
           "{\"ts\":%lu,\"type\":\"%s\",\"sev\":\"%s\",\"src\":\"%s\",\"asset\":\"%s\"%s%s}",
           (unsigned long)e.ts, toString(type), sev == Severity::INFO ? "INFO"
           : sev == Severity::NOTICE ? "NOTICE"
           : sev == Severity::WARNING ? "WARNING" : "CRITICAL",
           e.source, e.assetId,
           e.payloadJson[0] ? ",\"payload\":" : "",
           e.payloadJson);
  persist(line);
}

void EventEngine::persist(const char* line) {
  if (_storage != nullptr) {
    _storage->appendEvent(line);
  }
  // Log de console p/ bancada (não entra em produção no volume máximo — ver P1)
  Serial.printf("[EVT] %s\n", line);
}

uint32_t EventEngine::count() const {
  return _eventCount;
}

bool EventEngine::latest(EventRecord& out) const {
  if (_eventCount == 0) return false;
  // Head aponta para o mais antigo do ring; o último gravado é head-1
  size_t idx = (_ringHead + RING_SIZE - 1) % RING_SIZE;
  out = _ring[idx];
  return true;
}
}
