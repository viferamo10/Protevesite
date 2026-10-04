#pragma once
#include "Types.h"
#include "KnowledgeEngine.h"
namespace proteve {
// OIE — Operational Intelligence Engine: o cérebro comum de TODAS as interfaces.
// Pipeline (§13): DETECTAR → CORRELACIONAR → DIAGNOSTICAR → CONFIANÇA →
// PREVER CONSEQUÊNCIA → CALCULAR IMPACTO → RECOMENDAR.
// O OIE NÃO ATUA DIRETAMENTE: recomendações vão ao Actuation via política
// autorizada (R-09). A IA apenas EXPLICA a decisão — não inventa (R-02).
struct Recommendation {
  float confidence; char assetId[12];
  char hypothesis[96]; char evidence[192]; char action[48];
};
class OIE {
public:
  void begin(class RuleEngine& r, class KnowledgeEngine& k, class LearningEngine& l,
             class AlertManager& a, class AssetManager& am, class TelemetryEngine& t);
  void tick(uint32_t now);
  Recommendation evaluate(const Snapshot& snap);
  // Interface para chat (Dashboard/Telegram): pergunta em NL → resposta com evidências
  bool answerQuery(const char* questionJson, char* outJson, size_t n);
};
}
