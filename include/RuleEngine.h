#pragma once
#include "Types.h"
#include "Storage.h"
namespace proteve {
// Regras estruturadas: {metric, comparator, threshold, window, severity, actions}.
// Entrada por configuração natural é sempre convertida em regra estruturada e
// auditada (INTERPRETAÇÃO → PROPOSTA → CONFIRMAÇÃO → EXECUÇÃO → AUDITORIA).
class EventEngine; class AlertManager;
struct RuleResult { bool fired; char ruleId[12]; char detail[96]; };
class RuleEngine {
public:
  void begin(Storage& s);
  void tick(uint32_t now, const Snapshot& snap);
  // "Me avise se o consumo ficar 15% acima do padrão" → regra versionada
  const char* proposeFromNaturalLanguage(const char* text, char* outJson, size_t n);
};
}
