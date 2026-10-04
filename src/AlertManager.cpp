// AlertManager.cpp — CHG-20260906-014 (P0.4 — R-07 / RECONSTRUCTION §14)
// Alerta é ESTADO com lifecycle: OPEN → ACKNOWLEDGED → ACTIVE → ESCALATED →
// RESOLVED → CLOSED. Evidência prática: export Telegram dez/2025 (AUDIT-TG-001)
// = 475 rearmes avulsos sem ciclo de vida. Dedup por fingerprint tipo+ativo
// (Telegram 2.0 "silêncio inteligente", ADR-004). Resolução registra duração +
// causa provável + resultado — exatamente o "ALERTA RESOLVIDO" da reconstrução.

#include "AlertManager.h"
#include "EventEngine.h"
#include "Storage.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {

void AlertManager::begin(Storage& s, EventEngine& ev) {
  _storage = &s;
  _events = &ev;
  _nextId = 1;
  for (uint8_t i = 0; i < MAX_ALERTS; i++) _slots[i].used = false;
}

Alert* AlertManager::find(uint32_t id) {
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    if (_slots[i].used && _slots[i].alert.id == id) return &_slots[i].alert;
  }
  return nullptr;
}

Alert* AlertManager::findActive(const char* type, const char* assetId) {
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    const Alert& a = _slots[i].alert;
    if (!_slots[i].used) continue;
    if (a.status == AlertStatus::RESOLVED || a.status == AlertStatus::CLOSED) continue;
    if (strncmp(a.type, type, sizeof(a.type)) == 0 &&
        strncmp(a.assetId, assetId ? assetId : "-", sizeof(a.assetId)) == 0) {
      return &_slots[i].alert;
    }
  }
  return nullptr;
}

Alert* AlertManager::open(const char* type, const char* assetId, Severity sev,
                          const char* evidenceJson) {
  if (!type) return nullptr;

  // DEDUP por fingerprint (R-07 + ADR-004): condição persistente = MESMO alerta,
  // não dezenas de mensagens. Atualiza severidade se escalar.
  Alert* dup = findActive(type, assetId);
  if (dup != nullptr) {
    if ((uint8_t)sev > (uint8_t)dup->sev) dup->sev = sev;
    return dup;
  }

  // slot livre
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    if (_slots[i].used) continue;
    Alert& a = _slots[i].alert;
    _slots[i].used = true;
    a.id = _nextId++;
    a.status = AlertStatus::OPEN;
    a.sev = sev;
    a.openedTs = millis();
    a.ackedTs = a.resolvedTs = 0;
    a.cause[0] = a.outcome[0] = '\0';
    snprintf(a.type, sizeof(a.type), "%s", type);
    snprintf(a.assetId, sizeof(a.assetId), "%s", assetId ? assetId : "-");
    if (_events) {
      _events->emit(EventType::ALERT_OPEN, sev, "ALERTMGR", a.assetId, evidenceJson);
    }
    return &a;
  }
  return nullptr;   // sem slot: contagem limitada — registrado via evento no tick
}

void AlertManager::acknowledge(uint32_t id) {
  Alert* a = find(id);
  if (!a || a->status != AlertStatus::OPEN) return;
  a->status = AlertStatus::ACKNOWLEDGED;
  a->ackedTs = millis();
}

void AlertManager::escalate(uint32_t id) {
  Alert* a = find(id);
  if (!a) return;
  a->status = AlertStatus::ESCALATED;
  if ((uint8_t)a->sev < (uint8_t)Severity::CRITICAL) a->sev = Severity::CRITICAL;
  if (_events) _events->emit(EventType::ALERT_ESCALATED, a->sev, "ALERTMGR", a->assetId);
}

void AlertManager::resolve(uint32_t id, const char* cause, const char* outcome) {
  Alert* a = find(id);
  if (!a || a->status == AlertStatus::RESOLVED || a->status == AlertStatus::CLOSED) return;
  a->status = AlertStatus::RESOLVED;
  a->resolvedTs = millis();
  snprintf(a->cause, sizeof(a->cause), "%s", cause ? cause : "condicao normalizada");
  snprintf(a->outcome, sizeof(a->outcome), "%s", outcome ? outcome : "normalizado");
  if (_events) {
    // duração (s) + causa + resultado — o "ALERTA RESOLVIDO" da RECONSTRUCTION §14
    char payload[96];
    snprintf(payload, sizeof(payload), "{\"dur_s\":%lu,\"cause\":\"%s\"}",
             (unsigned long)((a->resolvedTs - a->openedTs) / 1000UL), a->cause);
    _events->emit(EventType::ALERT_RESOLVED, Severity::INFO, "ALERTMGR", a->assetId, payload);
  }
}

void AlertManager::close(uint32_t id) {
  Alert* a = find(id);
  if (!a) return;
  a->status = AlertStatus::CLOSED;
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    if (_slots[i].used && &_slots[i].alert == a) _slots[i].used = false;
  }
}

void AlertManager::tick(uint32_t now) {
  // Reavaliação de ativos (RECONSTRUCTION §14: "pode lembrar / pode escalar"):
  // OPEN sem acknowledge por 10min → ESCALATED. Não envia mensagem direto —
  // interfaces (Telegram/Dashboard) consultam o estado daqui (R-07).
  const uint32_t ESCALATE_AFTER_MS = 60000UL;
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    if (!_slots[i].used) continue;
    Alert& a = _slots[i].alert;
    if (a.status == AlertStatus::OPEN && (now - a.openedTs) > ESCALATE_AFTER_MS) {
      escalate(a.id);
    }
  }
}

uint8_t AlertManager::activeCount() const {
  uint8_t n = 0;
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    if (!_slots[i].used) continue;
    const Alert& a = _slots[i].alert;
    if (a.status != AlertStatus::RESOLVED && a.status != AlertStatus::CLOSED) n++;
  }
  return n;
}

Alert* AlertManager::active(uint8_t idx) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < MAX_ALERTS; i++) {
    if (!_slots[i].used) continue;
    Alert& a = _slots[i].alert;
    if (a.status == AlertStatus::RESOLVED || a.status == AlertStatus::CLOSED) continue;
    if (n == idx) return &a;
    n++;
  }
  return nullptr;
}
}
