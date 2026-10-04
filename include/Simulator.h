#pragma once
#include "Types.h"
namespace proteve {
class SensorManager; class TelemetryEngine;
// Simulador para testes (câmara fria, perda de sensor, queda de energia…).
// TODO: TODA amostra gerada carrega origin=SIMULATED — jamais misturada com REAL (R-06).
class Simulator {
public:
  void begin(SensorManager& sm, TelemetryEngine& t);
  void setEnabled(bool on);
  void tick(uint32_t now);
  void scenario_SensorLoss(); void scenario_SlowRecovery(); void scenario_DemandPeak();
};
}
