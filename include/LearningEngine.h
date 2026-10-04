#pragma once
#include <Arduino.h>
#include "Types.h"

// ═══════════════════════════════════════════════════════════════════════════
// LearningEngine — Learning Machine do V11.9 portada (CHG-20260906-005)
// PORTE FIEL de: lmUpdate (L586-620), atualizarRegistro/verificarDegradacao
// (L487-514), dois alphas confirmados na auditoria (0,08 instalação / 0,15
// por equipamento), gate de treinamento (≥8 amostras/hora, baseP>10W),
// anomalia +28%, degradação 1,20× corrente e FP −0,12 & <0,75.
//
// CORREÇÃO LEGACY-BUG-001 (AUDIT-V11.9.md): no V11.9 o rollover diário
// (energia de ontem) estava aninhado no bloco de relatório 8h/20h → inalcançável.
// Aqui existe dayRollover() EXPLÍCITO, chamado por quem detecta a virada de
// dia (TelemetryEngine/RTC) — fora de qualquer bloco de relatório.
// ═══════════════════════════════════════════════════════════════════════════

namespace proteve {

struct InstallationLM {
  float baseP[24] = {0}, baseI[24] = {0}, basePF[24] = {0};
  uint16_t amostras[24] = {0};
  bool treinado = false;
  TipoAnomalia anomalia = TipoAnomalia::AN_NENHUMA;
  float desvioPct = 0;
  char descAnomalia[80] = "";
  char contexto[16] = "—";
  float energiaHoje = 0, custoHoje = 0, energiaOntem = 0, custoOntem = 0;
  uint32_t surtos = 0, quedas = 0, faltas = 0, eventosUso = 0, minutosAtivo = 0;
};

struct EquipmentRecord {         // RegistroEquip do V11.9
  float corrMediaHist = 0, pfMedioHist = 0;
  uint16_t amostras = 0;
};

struct DegradationAdvice {       // "Conselho" do V11.9 (formato reduzido)
  bool active = false;
  char title[48] = "", text[200] = "";
  uint8_t tipo = 4;              // cooldown COOLDOWN_TIPO[4]=60min no V11.9
};

class LearningEngine {
public:
  void begin() { /* persistência (SD/Asset DNA): P1 */ }

  // ---- baseline da instalação (lmUpdate, V11.9 L586-600) ----
  void updateBaseline(uint8_t hour, float pTot, float iTot, float pfAvg);
  bool isTrained() const { return _lm.treinado; }
  const InstallationLM& lm() const { return _lm; }

  // ---- energia/custo incremental (lmUpdate, V11.9 L602-604) ----
  // tariff = tarifa efetiva (base + bandeira) — R-14: configurável por instalação.
  void addEnergy(float pTot, float secondsSinceLast, float tariff, uint8_t periodIdx);
  // CHG-20260906-005: correção do LEGACY-BUG-001 — rollover diário explícito.
  void dayRollover();

  // ---- anomalia de consumo (lmUpdate, V11.9 L607-608: gate ≥8 amostras, baseP>10W) ----
  void checkAnomaly(uint8_t hour, float pTot);

  // ---- registro por equipamento (atualizarRegistro, V11.9 L487-492; alpha=0.15) ----
  void updateEquipmentRecord(int equipIdx, float corr, float pf);
  const EquipmentRecord& record(int equipIdx) const;

  // ---- degradação (verificarDegradacao, V11.9 L494-514; gate ≥5 amostras) ----
  DegradationAdvice checkDegradation(const char* nome, int equipIdx, float corr, float pf) const;

  // ---- parâmetros (R-14: históricos do V11.9 — todos configuráveis, não leis) ----
  float alphaInst   = 0.08f;    // LM_ALPHA (instalação/hora)
  float alphaEquip  = 0.15f;    // por equipamento (atualizarRegistro)
  float desvioAlerta = 0.28f;   // LM_DESVIO_ALERTA (+28%)
  uint16_t gateAmostras = 8;    // antes de alertar anomalia
  float gateBaseP = 10.0f;      // W — evita falso positivo em carga leve
  uint16_t gateEquipAmostras = 5; // antes de alertar degradação
  float degrCorrMult = 1.20f;   // corrente > 1,20× média histórica
  float degrPfQueda = 0.12f;    // FP caiu > 0,12 abaixo do histórico
  float degrPfMin = 0.75f;      // e FP < 0,75

private:
  InstallationLM _lm;
  EquipmentRecord _records[24];  // por equipIdx (ver NilmDatabase::N_EQUIP)
  static constexpr int MAX_RECORDS = 24;
};
}
