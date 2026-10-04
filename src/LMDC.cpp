// LMDC.cpp — CHG-20260906-019 — STUB HONESTO (P1, ADR-004)
// Load Management & Demand Control. Cargas por CRITICIDADE/circuito (K1-K4),
// shedding em 13 passos (ADR-004 §2). MVP: MODO MONITOR APENAS (ADR-003) —
// nenhuma carga é cortada por software no piloto da câmara fria.

#include "LMDC.h"
#include "AssetManager.h"
#include "Actuation.h"
#include "AlertManager.h"
#include <Arduino.h>

namespace proteve {
void LMDC::begin(AssetManager& am, Actuation& act, AlertManager& al) {
  (void)am; (void)act; (void)al;
  // TODO(P1): guardar refs; demandRisk() abre alerta RISK (não corta carga)
}
void LMDC::setMode(LmdcMode m) {
  // ADR-003: até a validação física completa o modo é MONITOR. Escalar de modo
  // exige política autorizada + feedback validado — não é decisão de software.
  (void)m;  // TODO(P1): persistir modo autorizado
}
void LMDC::tick(uint32_t now) { (void)now; /* TODO(P1): demanda + prioridades */ }
float LMDC::demandA() const { return 0.0f; }        // TODO(P1): do TelemetryEngine
bool LMDC::demandRisk() const { return false; }     // TODO(P1): 13 passos (ADR-004)
}
