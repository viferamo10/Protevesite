// Maintenance.cpp — CHG-20260906-020 — STUB FUNCIONAL MÍNIMO (P0.5 parcial)
// Cada intervenção vira evento (RECONSTRUCTION §33); o LearningEngine depois
// compara antes×depois da manutenção. Registro completo (técnico/peça/custo)
// entra com o banco estruturado (P0.1).

#include "Maintenance.h"
#include "EventEngine.h"
#include "Storage.h"
#include "LearningEngine.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {
void Maintenance::begin(Storage& s, EventEngine& ev, LearningEngine& le) {
  (void)s; (void)le;   // TODO(P0.1): persistir registros; comparar antes/depois
  _events = &ev;
}
void Maintenance::registerRecord(const MaintenanceRecord& r) {
  if (_events) {
    char payload[96];
    snprintf(payload, sizeof(payload), "{\"asset\":\"%s\",\"diag\":\"%s\",\"outcome\":\"%s\"}",
             r.assetId, r.diagnosis, r.outcome);
    _events->emit(EventType::MAINTENANCE, Severity::NOTICE, "MAINT", r.assetId, payload);
  }
  // TODO(P0.1): persistir o registro completo + marcar baseline p/ re-treino
}
void Maintenance::tick(uint32_t now) { (void)now; }
}
