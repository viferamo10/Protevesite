#pragma once
#include "Types.h"
#include "Storage.h"
namespace proteve {
class EventEngine;
// Alert Lifecycle (P0.4, R-07): alerta é ESTADO, não mensagem.
// OPEN → ACKNOWLEDGED → ACTIVE → ESCALATED → RESOLVED → CLOSED.
// Enquanto a condição persiste: pode lembrar, escalar, aparecer em Dashboard e Telegram.
struct Alert {
  uint32_t id; AlertStatus status; Severity sev;
  char assetId[12]; char type[24];
  uint32_t openedTs, ackedTs, resolvedTs;
  char cause[64]; char outcome[32];
};
class AlertManager {
public:
  void begin(Storage& s, EventEngine& ev);
  Alert* open(const char* type, const char* assetId, Severity sev, const char* evidenceJson);
  void acknowledge(uint32_t id);
  void escalate(uint32_t id);
  void resolve(uint32_t id, const char* cause, const char* outcome); // duração + causa + resultado
  void close(uint32_t id);
  void tick(uint32_t now);  // reavalia ativos → lembretes/escalação
  uint8_t activeCount() const; Alert* active(uint8_t idx);

private:
  static constexpr uint8_t MAX_ALERTS = 12;
  struct AlertSlot { Alert alert; bool used = false; };
  AlertSlot _slots[MAX_ALERTS];
  uint32_t _nextId = 1;
  Storage* _storage = nullptr;
  EventEngine* _events = nullptr;
  Alert* find(uint32_t id);
  Alert* findActive(const char* type, const char* assetId);   // dedup por fingerprint (ADR-004)
};
}
