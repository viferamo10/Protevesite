#pragma once
#include <Arduino.h>
#include "Types.h"

// ═══════════════════════════════════════════════════════════════════════════
// ProtectionEngine — FSM de religamento progressivo (CHG-20260906-002)
// PORTE FIEL do V11.9 (fsm(), L621-726). Fluxo:
//   ST_OK → anomalia → ST_DESARME (30s) → ST_ESTABIL (3 leituras OK de 10s)
//         → ST_REARME → ST_OK | 3 anomalias/15min → ST_LOCKOUT (5/10/15min)
//   ST_LOCKOUT → testa rede a cada 30s (antecipa se estável) | timer → ST_ESTABIL
//   ST_MANUAL → operador no controle (tick não atua)
//
// DIFERENÇAS INTENCIONAIS vs V11.9 (documentadas no EXTRACT-LOG):
//   1. Hardware desacoplado: a contatora é acionada via callback (ContactorFn),
//      nunca digitalWrite direto — integração com Actuation/feedback físico (R-09).
//   2. R-19 (fail-safe): o callback é chamado com `energize=false` em TODA
//      transição para DESARME/LOCKOUT — perda de energia de controle deixa a
//      contatora aberta por padrão (circuito NA no hardware).
//   3. R-18: este engine NÃO rearma disjuntor de potência (nunca existiu no
//      V11.9 e nunca vai existir aqui — religamento é só da contatora).
//   4. Detalhe da anomalia (surto/queda/falta p/ contadores do LM) é
//      responsabilidade do chamador/classificador — aqui fica o estado puro.
// ═══════════════════════════════════════════════════════════════════════════

namespace proteve {

class ProtectionEngine {
public:
  using LogFn       = void (*)(const char* level, const char* msg);
  using ContactorFn = void (*)(bool energize);   // R-19: circuito NA no hardware

  void begin(ContactorFn contactor, LogFn log) { _contactor = contactor; _log = log; }
  void setManual(bool m);                        // ST_MANUAL (V11.9 modoManual)
  void releaseLockout();                         // destrava lockout p/ re-tentar (R-18: só lockout)
  void tick(uint32_t now, bool defeituosa, bool estavel);

  // ---- estado (para Dashboard/API/Telegram) ----
  EstadoProt state() const { return _estado; }
  uint8_t   lockoutLevel() const { return _lockoutNivel; }
  uint16_t  rearmeWindowCount() const { return _rearmeCount; }
  uint32_t  lockoutRemainingMs(uint32_t now) const;
  bool      isEnergized() const { return _relayOn; }

  // ---- parâmetros (R-14: padrões históricos do V11.9, todos configuráveis) ----
  uint32_t tRearmeMs        = 30000UL;   // V11.9 T_REARME_MS (valor histórico; a citação "IEC 60255-11" do legacy é ERRADA — ver NORMS-001)
  uint32_t tJanelaMs        = 900000UL;   // 15 min (V11.9 T_JANELA_MS)
  uint8_t  maxRearmes       = 3;          // 3 anomalias → lockout (V11.9 MAX_REARMES)
  uint32_t tLockoutMs[4]    = {0, 300000UL, 600000UL, 900000UL}; // 5/10/15 min
  uint32_t tTesteLockMs     = 30000UL;    // teste de rede no lockout (V11.9)
  uint8_t  nLeitEstaveis    = 3;          // leituras OK antes de rearmar
  uint32_t tLeitEstaveisMs  = 10000UL;    // 10s entre leituras
  uint32_t tResetNivelMs    = 3600000UL;  // 60min sem ocorrências → zera nível

  // contadores de ciclo (o V11.9 mantinha cntRearmes/cntLockouts globais)
  uint32_t cntRearmes = 0, cntLockouts = 0;

private:
  void contactor(bool energize);          // R-19: energia=false sempre no desarme
  void logf(const char* level, const char* msg) { if (_log) _log(level, msg); }

  EstadoProt _estado = EstadoProt::ST_OK;
  bool   _modoManual = false;
  bool   _relayOn = false;
  uint8_t _lockoutNivel = 0;
  uint16_t _rearmeCount = 0;
  uint8_t _leitEstaveisOk = 0;
  uint32_t _tDesarme = 0, _tJanela = 0, _tUltimaAnomalia = 0;
  uint32_t _tLeitEst = 0, _tLockout = 0, _tTesteRede = 0;

  ContactorFn _contactor = nullptr;
  LogFn       _log = nullptr;
};
}
