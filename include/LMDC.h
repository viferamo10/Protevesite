#pragma once
#include "Types.h"
namespace proteve {
class AssetManager; class Actuation; class AlertManager;
// LMDC — Load Management & Demand Control (§17/§19/§41).
// MONITOR: nada controla. ASSISTED: recomenda + usuário autoriza.
// AUTOMATIC: só políticas pré-autorizadas, respeitando por carga:
// criticality, operational_priority, maximum_interrupt_time, restart_delay,
// shedding_allowed, automatic_control_allowed. Prioridade NUNCA é fixa (§20).
class LMDC {
public:
  void begin(AssetManager& am, Actuation& act, AlertManager& al);
  void setMode(LmdcMode m);   // default: MONITOR até validação completa
  void tick(uint32_t now);
  // Ex.: risco de sobrecarga térmica do quadro (corrente↑ + temp painel↑ + pico)
  // → shedding EV primeiro (baixa prioridade) → confirmar auxiliar → registrar.
private:
  float demandA() const; bool demandRisk() const;
};
}
