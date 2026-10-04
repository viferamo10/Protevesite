#pragma once
#include "Types.h"
#include <FS.h>
#include <SD.h>
#include <LittleFS.h>

namespace proteve {
// Local-first (R-12): MicroSD (HW-125) + NVS.
// Eventos/telemetria append-only em JSONL; configuração versionada; ring buffer
// para sobreviver a queda de energia. Futuro: ProteveSync → API → PostgreSQL.
class Storage {
public:
  bool begin();                     // SD + (RTC DS3231 via caller)
  bool appendEvent(const char* line);
  bool appendTelemetry(const char* line);
  bool readConfig(char* out, size_t n); bool writeConfig(const char* json);
  bool healthy() const;             // card presente + espaço

private:
  bool _sdPresent = false;
  bool _fsPresent = false;
  uint8_t _sdCsPin = 10;
};
}
