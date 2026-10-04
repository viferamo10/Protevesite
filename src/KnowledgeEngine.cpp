// KnowledgeEngine.cpp — CHG-20260906-018 — STUB HONESTO (P1)
// KB-001..KB-015 (RECONSTRUCTION §34): motores, compressores, refrigeração,
// qualidade, falhas, manutenção, knowledge graph, benchmark. A IA explica a
// decisão com evidências — não inventa (R-02, §35).

#include "KnowledgeEngine.h"
#include "Storage.h"
#include <Arduino.h>

namespace proteve {
void KnowledgeEngine::begin(Storage& s) { (void)s; /* TODO(P1): KB-JSON do repo */ }
uint8_t KnowledgeEngine::hypothesesFor(const char* symptom, const Snapshot& ctx,
                                        Hypothesis* out, uint8_t max) {
  (void)symptom; (void)ctx; (void)out; (void)max;
  return 0;   // TODO(P1): multi-hipótese com confiança (§32)
}
}
