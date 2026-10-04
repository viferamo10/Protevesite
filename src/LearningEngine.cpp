// LearningEngine.cpp — CHG-20260906-005
// Porte fiel do V11.9 (lmUpdate/atualizarRegistro/verificarDegradacao).
// Toda alteração exige CHANGE-ID (R-13). Parâmetros são defaults históricos (R-14).
#include "LearningEngine.h"
#include <stdio.h>

namespace proteve {

void LearningEngine::updateBaseline(uint8_t hour, float pTot, float iTot, float pfAvg) {
  if (hour > 23) return;
  // V11.9 L590-591: primeira amostra da hora inicializa; depois EMA com LM_ALPHA
  if (_lm.amostras[hour] == 0) {
    _lm.baseP[hour] = pTot; _lm.baseI[hour] = iTot; _lm.basePF[hour] = pfAvg;
  } else {
    _lm.baseP[hour]  = (1 - alphaInst) * _lm.baseP[hour]  + alphaInst * pTot;
    _lm.baseI[hour]  = (1 - alphaInst) * _lm.baseI[hour]  + alphaInst * iTot;
    _lm.basePF[hour] = (1 - alphaInst) * _lm.basePF[hour] + alphaInst * pfAvg;
  }
  _lm.amostras[hour]++;

  // V11.9 L592: treinado quando ≥12 horas com ≥24 amostras cada
  if (!_lm.treinado) {
    int ht = 0;
    for (int i = 0; i < 24; i++) if (_lm.amostras[i] >= 24) ht++;
    _lm.treinado = (ht >= 12);
  }
}

void LearningEngine::addEnergy(float pTot, float secondsSinceLast, float tariff, uint8_t periodIdx) {
  // V11.9 L602-604: energia incremental do intervalo + custo + minuto ativo
  float dE = pTot * secondsSinceLast / 3600.0f;   // kWh
  _lm.energiaHoje += dE;
  _lm.custoHoje += dE * tariff;
  _lm.minutosAtivo++;
  (void)periodIdx;  // acumulação por período (PeriodoStats) entra com o TelemetryEngine (P0.1)
}

void LearningEngine::dayRollover() {
  // CHG-20260906-005 — CORREÇÃO DO LEGACY-BUG-001 (AUDIT-V11.9.md):
  // no V11.9 este bloco estava dentro de if((h==8||h==20)…) → if(h==0) inalcançável.
  // Aqui é método explícito, chamado pelo TelemetryEngine na virada de dia (meia-noite).
  _lm.energiaOntem = _lm.energiaHoje;
  _lm.custoOntem = _lm.custoHoje;
  _lm.energiaHoje = 0; _lm.custoHoje = 0;
}

void LearningEngine::checkAnomaly(uint8_t hour, float pTot) {
  // V11.9 L607-608: gate anti-falso-positivo (≥8 amostras na hora E baseP>10W)
  _lm.anomalia = TipoAnomalia::AN_NENHUMA;
  _lm.desvioPct = 0;
  _lm.descAnomalia[0] = '\0';
  if (hour > 23) return;
  if (_lm.amostras[hour] >= gateAmostras && _lm.baseP[hour] > gateBaseP) {
    float dev = (pTot - _lm.baseP[hour]) / _lm.baseP[hour];
    _lm.desvioPct = dev * 100.0f;
    if (dev > desvioAlerta) {
      _lm.anomalia = TipoAnomalia::AN_CONSUMO_ALTO;
      snprintf(_lm.descAnomalia, sizeof(_lm.descAnomalia),
               "Consumo %.0fW (+%.0f%% acima do esperado)", pTot, dev * 100);
    }
  }
}

void LearningEngine::updateEquipmentRecord(int equipIdx, float corr, float pf) {
  if (equipIdx < 0 || equipIdx >= MAX_RECORDS) return;
  EquipmentRecord& r = _records[equipIdx];
  // V11.9 L487-492 (atualizarRegistro): primeira amostra inicializa; depois EMA α=0,15
  if (r.amostras == 0) {
    r.corrMediaHist = corr; r.pfMedioHist = pf;
  } else {
    r.corrMediaHist = (1 - alphaEquip) * r.corrMediaHist + alphaEquip * corr;
    r.pfMedioHist   = (1 - alphaEquip) * r.pfMedioHist  + alphaEquip * pf;
  }
  r.amostras++;
}

const EquipmentRecord& LearningEngine::record(int equipIdx) const {
  static EquipmentRecord zero{};
  if (equipIdx < 0 || equipIdx >= MAX_RECORDS) return zero;
  return _records[equipIdx];
}

DegradationAdvice LearningEngine::checkDegradation(const char* nome, int equipIdx,
                                                   float corr, float pf) const {
  // V11.9 L494-514 (verificarDegradacao): gate ≥5 amostras; dois critérios
  DegradationAdvice d;
  if (equipIdx < 0 || equipIdx >= MAX_RECORDS) return d;
  const EquipmentRecord& r = _records[equipIdx];
  if (r.amostras < gateEquipAmostras) return d;

  if (corr > r.corrMediaHist * degrCorrMult) {           // corrente > 1,20× média
    d.active = true; d.tipo = 4;
    snprintf(d.title, sizeof(d.title), "Atencao: %s", nome);
    snprintf(d.text, sizeof(d.text),
             "O %s esta consumindo %.1fA — acima do habitual (media %.1fA). "
             "Pode indicar desgaste ou contato frouxo. Vale uma revisao preventiva.",
             nome, corr, r.corrMediaHist);
    return d;
  }
  if (pf < r.pfMedioHist - degrPfQueda && pf < degrPfMin) {   // FP −0,12 e <0,75
    d.active = true; d.tipo = 4;
    snprintf(d.title, sizeof(d.title), "Revisao recomendada: %s", nome);
    snprintf(d.text, sizeof(d.text),
             "O fator de potencia do %s caiu para %.2f — historico: %.2f. "
             "Pode indicar capacitor enfraquecendo ou conexoes oxidadas.",
             nome, pf, r.pfMedioHist);
  }
  return d;
}
}
