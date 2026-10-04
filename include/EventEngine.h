#pragma once
#include "Types.h"
#include "Storage.h"

namespace proteve {
// Event Engine (P0.3): TUDO vira evento, append-only (R-08).
// Persistência edge: JSONL no MicroSD + ring buffer RAM. Fonte da verdade operacional.

struct EventRecord {
  uint32_t ts = 0;
  EventType type = EventType::BOOT;
  Severity sev = Severity::INFO;
  char source[16] = {0};
  char assetId[16] = {0};
  char payloadJson[64] = {0};
};

class EventEngine {
public:
  void begin(Storage& storage);
  void emit(EventType type, Severity sev, const char* source,
            const char* assetId = nullptr, const char* payloadJson = nullptr);
  uint32_t count() const;
  bool latest(EventRecord& out) const;
  // Consulta para OIE/Dashboard/Telegram/relatórios (por tipo, janela, ativo)
  // TODO(P0.3): iterador de replay para diagnóstico causal histórico

private:
  void persist(const char* line);

  Storage* _storage = nullptr;
  uint32_t _eventCount = 0;

  static const size_t RING_SIZE = 32;
  EventRecord _ring[RING_SIZE];
  size_t _ringHead = 0;
};
}
