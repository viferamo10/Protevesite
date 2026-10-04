// OIE.cpp — CHG-20260906-016 — STUB HONESTO (P1)
// Pipeline maduro (RECONSTRUCTION §13): DETECTAR→CORRELACIONAR→DIAGNOSTICAR→
// CONFIANÇA→PREVER CONSEQUÊNCIA→CALCULAR IMPACTO→RECOMENDAR. O OIE NÃO ATUA:
// recomendações passam pelo Policy Engine/Actuation (R-09, ADR-004).
// Este stub apenas liga as referências para o kernel compilar; a lógica de
// correlação multi-hipótese (§32: recuperação lenta × porta aberta × compressor)
// entra em P1 sobre TelemetryEngine + LearningEngine + KnowledgeEngine.

#include "OIE.h"
#include "RuleEngine.h"
#include "LearningEngine.h"
#include "AlertManager.h"
#include "AssetManager.h"
#include "TelemetryEngine.h"
#include <Arduino.h>

namespace proteve {

void OIE::begin(RuleEngine& r, KnowledgeEngine& k, LearningEngine& l,
                AlertManager& a, AssetManager& am, TelemetryEngine& t) {
  // Referências guardadas via ponteiros privados quando a lógica P1 entrar.
  (void)r; (void)k; (void)l; (void)a; (void)am; (void)t;
}

void OIE::tick(uint32_t now) {
  (void)now;  // TODO(P1): consumir anomalias do LearningEngine e abrir alerts
}

Recommendation OIE::evaluate(const Snapshot& snap) {
  Recommendation r{};
  r.confidence = 0.0f;
  r.hypothesis[0] = r.evidence[0] = r.action[0] = r.assetId[0] = '\0';
  (void)snap;  // TODO(P1): correlacionar temp/porta/corrente/FP (§31/§32)
  return r;
}

bool OIE::answerQuery(const char* questionJson, char* outJson, size_t n) {
  (void)questionJson;
  snprintf(outJson, n, "{\"error\":\"OIE P1 nao implementado\"}");
  return false;   // TODO(P1): Chat Operacional (ADR-004 §14/§19)
}
}
