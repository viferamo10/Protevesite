// RuleEngine.cpp — CHG-20260906-017 — STUB HONESTO (P1)
// Regras estruturadas (métrica, comparador, limite, janela, severidade, ação).
// Configuração por linguagem natural (RECONSTRUCTION §39): NL → regra estruturada
// → CONFIRMAÇÃO → EXECUÇÃO → AUDITORIA. Nada de regra burlando R-09 (§40).

#include "RuleEngine.h"
#include "Storage.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {
void RuleEngine::begin(Storage& s) { (void)s; /* TODO(P1): carregar rules.json */ }
void RuleEngine::tick(uint32_t now, const Snapshot& snap) {
  (void)now; (void)snap;  // TODO(P1): avaliar regras contra telemetria
}
const char* RuleEngine::proposeFromNaturalLanguage(const char* text, char* outJson, size_t n) {
  (void)text;
  snprintf(outJson, n, "{\"error\":\"NL→regra: P1\"}");
  return nullptr;
}
}
