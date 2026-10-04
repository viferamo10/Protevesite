#pragma once
#include "Types.h"
#include "Storage.h"
namespace proteve {
// Asset DNA (P0.2): cada equipamento tem ID permanente (AST-000001) + prontuário.
// Inclui políticas LMDC por carga (§20): criticality, operational_priority,
// maximum_interrupt_time, restart_delay, shedding_allowed, automatic_control_allowed.
// O DNA operacional acumulado (horas, consumo/fp histórico, degradação, risco,
// prob. de falha) é DERIVADO de telemetry/events/maintenance — nunca duplicado.
class AssetManager {
public:
  void begin(Storage& storage);
  bool load(); bool save();
  const char* createAsset(const char* name, const char* type, const char* location);
  // Correção humana de identificação NILM vira conhecimento (RECONSTRUCTION §9)
  void applyHumanCorrection(const char* inferredAssetId, const char* realAssetId,
                            const char* evidenceJson);
  // Consultas: byId, byLocation (Energy Map), loadsOrderedBySheddingPriority()
  const char* byId(const char* id) const;   // nome do ativo (ou nullptr)

private:
  static constexpr uint8_t MAX_ASSETS = 24;
  struct AssetRec {
    char id[12]; char name[32]; char type[16]; char location[24];
    Criticality criticality = Criticality::CT_MED;
    bool sheddingAllowed = false;         // §20: nem toda carga pode ser cortada
    bool automaticControlAllowed = false; // MVP: false p/ TODAS (ADR-003 ALARM-ONLY)
    uint8_t operationalPriority = 3;      // 0=mais alta
  };
  AssetRec _assets[MAX_ASSETS];
  uint8_t _count = 0;
  Storage* _storage = nullptr;
};
}
