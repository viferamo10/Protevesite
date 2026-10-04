// Simulator.cpp — CHG-20260906-021 — STUB HONESTO (testes §59.20-24)
// Cenários de FALHA exigidos pela reconstrução: perda de sensor, perda de Wi-Fi,
// queda de energia, recuperação, pico de demanda. Dados gerados são SEMPRE
// marcados DataOrigin::SIMULATED e NUNCA treinam o baseline real (R-06 — o
// TelemetryEngine já ignora snapshot SIMULATED no learning por gate de origem).

#include "Simulator.h"
#include "SensorManager.h"
#include "TelemetryEngine.h"
#include <Arduino.h>

namespace proteve {
void Simulator::begin(SensorManager& sm, TelemetryEngine& t) {
  (void)sm; (void)t;   // TODO(P0-tests): injetar snapshots SIMULATED marcados
}
void Simulator::setEnabled(bool on) { (void)on; }
void Simulator::tick(uint32_t now) { (void)now; }
void Simulator::scenario_SensorLoss() { /* TODO(P0-tests): sensor OFFLINE→STALE */ }
void Simulator::scenario_SlowRecovery() { /* TODO(P0-tests): §31 recuperação lenta */ }
void Simulator::scenario_DemandPeak() { /* TODO(P0-tests): simultaneidade §41 */ }
}
