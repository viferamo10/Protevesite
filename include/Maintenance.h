#pragma once
#include "Types.h"
#include "Storage.h"
namespace proteve {
class EventEngine; class LearningEngine;
// Manutenção vira EVENTO (§33): problema, diagnóstico, técnico, empresa, peça,
// custo, tempo, resultado. Depois o sistema aprende o ANTES e o DEPOIS —
// cria cadeia causal histórica ("após manutenção, consumo voltou ao padrão").
struct MaintenanceRecord {
  char assetId[12]; uint32_t ts; char problem[64]; char diagnosis[64];
  char technician[32]; char company[32]; char part[32];
  float cost; uint16_t durationMin; char outcome[24];
};
class Maintenance {
public:
  void begin(Storage& s, EventEngine& ev, LearningEngine& le);
  void registerRecord(const MaintenanceRecord& r);
  void tick(uint32_t now);

private:
  EventEngine* _events = nullptr;
  Storage* _storage = nullptr;      // TODO(P0.1): registros persistentes
  LearningEngine* _learning = nullptr;  // TODO(P0.1): antes×depois da manutenção
};
}
