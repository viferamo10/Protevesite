#pragma once
#include "Types.h"
namespace proteve {
// ============================================================================
// MCSAEngine — STUB / ESPECIFICADO, NÃO IMPLEMENTADO (ver PEMB-016).
// Motor Current Signature Analysis: FFT da corrente (SCT-013) correlacionada
// com temperatura de carcaça (DS18B20 por motor) para diagnóstico diferencial
// de falhas mecânicas/elétricas em motores de indução (ex.: compressor da
// câmara fria). NÃO está no hardware do primeiro cliente — candidato a
// P1.5/P2, requer SCT-013 + ADR de pinagem própria no S3 antes de ligar
// qualquer GPIO. Roda em ciclo próprio (ex.: Core 0, 10s) e NUNCA pode
// competir com a tarefa de proteção elétrica (R-04) nem com a leitura dos
// PZEM/Asset DNA já em produção.
// ============================================================================
struct MCSAResult {
  bool valido = false;
  float thdPct = 0;
  float freqPico = 0;
  float sidebandDb = 0;       // severidade relativa ao pico fundamental
  DataOrigin origin = DataOrigin::REAL;
};

enum class MotorFaultHypothesis : uint8_t {
  SAUDAVEL, SOBRECARGA_MECANICA, FALHA_VENTILACAO, BARRAS_ROTOR_QUEBRADAS,
  DEGRADACAO_ESTATOR, CAPACITOR_DEGRADADO, CURTO_ENTRE_ESPIRAS, DESEQUILIBRIO_FASES
};

class MCSAEngine {
public:
  // Placeholder de interface — corpo NÃO implementado (PEMB-016 é especificação).
  void begin() { /* TODO(P1.5/P2): não ligar sem ADR de pinagem do SCT-013 no S3 */ }
  bool sampleAndAnalyze(MCSAResult& out) { (void)out; return false; }
  // Diagnóstico diferencial corrente×temperatura — hipótese + confiança (R-02),
  // nunca certeza. Correlacionar com LearningEngine::degradationRate() do motor.
  MotorFaultHypothesis correlate(const MCSAResult& mcsa, float tempC, float tempTrendCPerMin) const {
    (void)mcsa; (void)tempC; (void)tempTrendCPerMin;
    return MotorFaultHypothesis::SAUDAVEL; // stub
  }
};
}
