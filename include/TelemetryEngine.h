#pragma once
#include "Types.h"
#include "Storage.h"
#include "SensorManager.h"
#include "LearningEngine.h"

namespace proteve {
class AssetManager;

// Tarifa e bandeiras tarifárias (CHG-20260906-006 - V11.9 L100-105)
constexpr float TARIFA_BASE_HISTORICA = 0.75f;
constexpr float ACRESCIMO_BAN[4] = { 0.0f, 0.01874f, 0.03971f, 0.09492f }; // Verde, Amarela, Verm1, Verm2

// Coleta, marca tempo (RTC DS3231), agrega e persiste telemetria.
// RESPEITA hierarquia de medidores pai/filho (R-11): nunca soma geral + filhos.
// Toda amostra carrega origem REAL/SIMULATED/DERIVED (R-06).
class TelemetryEngine {
public:
  void begin(Storage& s, SensorManager& sm, AssetManager& am);
  void setLearningEngine(LearningEngine* le);
  void tick(uint32_t now);
  bool latest(uint8_t channel, Snapshot& out) const;
  float totalLoadW() const;        // consumo da instalação sem dupla contagem
  float effectiveTariff() const;   // TARIFA_BASE + ACRESCIMO_BAN (CHG-006)
  void setBandeira(uint8_t b);     // 0=Verde, 1=Amarela, 2=Verm1, 3=Verm2
  uint8_t bandeira() const { return _bandeira; }
  // Abstração solar/futura: GRID_IMPORT, GRID_EXPORT, SOLAR_GENERATION, HOUSE_LOAD

private:
  Storage* _storage = nullptr;
  SensorManager* _sensors = nullptr;
  AssetManager* _assets = nullptr;
  LearningEngine* _learning = nullptr;

  uint8_t _bandeira = 0; // BAN_VERDE
  uint32_t _lastTickMs = 0;
  uint32_t _lastLmMs = 0;
  uint8_t _lastDayHour = 255;
};
}
