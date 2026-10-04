#pragma once
#include "Types.h"
namespace proteve {
class Storage;
// Knowledge Base KB-001..015 (motores, compressores, refrigeração, qualidade,
// falhas elétricas/mecânicas, proteções, sensores, ocorrências, custos,
// manutenção, regras, knowledge graph, benchmark, aprendizado).
// Fornece evidências e gera HIPÓTESES MÚLTIPLAS — nunca diagnóstico único (R-02).
// TODO(P1): carregamento do knowledge-base/ do repo (SD) + knowledge graph local.
struct Hypothesis { char text[96]; float confidence; char evidence[160]; };
class KnowledgeEngine {
public:
  void begin(Storage& s);
  uint8_t hypothesesFor(const char* symptom, const Snapshot& ctx,
                         Hypothesis* out, uint8_t max);
};
}
