// ProtectionEngine.cpp — CHG-20260906-002
// Porte fiel da FSM do V11.9 (L621-726). Toda desvio intencional está marcado
// no header e no EXTRACT-LOG. Não otimizar sem CHANGE-ID (R-13).
#include "ProtectionEngine.h"
#include <stdio.h>

namespace proteve {

void ProtectionEngine::setManual(bool m) {
  if (m && !_modoManual) {
    logf("WARN", "Modo MANUAL: protecao automatica suspensa (ST_MANUAL)");
    _estado = EstadoProt::ST_MANUAL;
  } else if (!m && _modoManual) {
    logf("INFO", "Modo AUTOMATICO restaurado");
    _estado = EstadoProt::ST_OK;
  }
  _modoManual = m;
}

void ProtectionEngine::releaseLockout() {
  // R-18: reset remoto destrava APENAS o lockout da contatora — nunca disjuntor
  // de potência (sempre manual). Volta ao fluxo de estabilidade como o V11.9
  // fazia ao sair do lockout.
  if (_estado == EstadoProt::ST_LOCKOUT) {
    logf("INFO", "Reset remoto: lockout destravado (contatora). Disjuntor de potencia NAO e rearmado por software (R-18)");
    _rearmeCount = 0; _leitEstaveisOk = 0;
    _tDesarme = millis();                 // aguarda novo ciclo de estabilidade
    _estado = EstadoProt::ST_ESTABIL;
  }
}

uint32_t ProtectionEngine::lockoutRemainingMs(uint32_t now) const {
  if (_estado != EstadoProt::ST_LOCKOUT) return 0;
  uint32_t dur = tLockoutMs[_lockoutNivel < 4 ? _lockoutNivel : 3];
  uint32_t el = now - _tLockout;
  return (el >= dur) ? 0 : (dur - el);
}

void ProtectionEngine::contactor(bool energize) {
  _relayOn = energize;
  if (_contactor) _contactor(energize);   // R-19: hardware NA — sem energia = aberto
}

void ProtectionEngine::tick(uint32_t now, bool defeituosa, bool estavel) {
  char buf[96];

  // Reset do nível de lockout após 60min sem ocorrências (V11.9 L623-625)
  if (_lockoutNivel > 0 && _tUltimaAnomalia > 0 &&
      (now - _tUltimaAnomalia) > tResetNivelMs) {
    _lockoutNivel = 0;
    logf("INFO", "Nivel de lockout resetado (60min sem anomalias)");
  }

  // Janela de rearmes: zera contador se passou 15min (V11.9 L627)
  if (_rearmeCount > 0 && (now - _tJanela) > tJanelaMs) _rearmeCount = 0;

  if (_modoManual) return;                // ST_MANUAL: não atua automaticamente (V11.9 L629)

  switch (_estado) {

    case EstadoProt::ST_OK:
      if (defeituosa) {
        contactor(false);                 // R-19: desarma (NA — abre)
        _estado = EstadoProt::ST_DESARME;
        _tDesarme = now; _tUltimaAnomalia = now;
        snprintf(buf, sizeof(buf), "DESARME — anomalia na rede. Aguardando %lus (retardo de religamento — prática de engenharia, ver NORMS-001)",
                 (unsigned long)(tRearmeMs / 1000));
        logf("ERRO", buf);
      }
      break;

    case EstadoProt::ST_DESARME:          // aguardando retardo de 30s (V11.9 L630-636)
      if ((now - _tDesarme) >= tRearmeMs) {
        _estado = EstadoProt::ST_ESTABIL;
        _leitEstaveisOk = 0; _tLeitEst = now;
        logf("INFO", "Aguardando estabilidade de rede (retardo de religamento — pratica de engenharia, ver NORMS-001)");
      }
      break;

    case EstadoProt::ST_ESTABIL:          // N leituras OK consecutivas (V11.9 L638-654)
      if (defeituosa) {
        _leitEstaveisOk = 0; _tDesarme = now;
        _estado = EstadoProt::ST_DESARME;
        logf("WARN", "Rede instavel durante verificacao — reiniciando delay");
      } else if ((now - _tLeitEst) >= tLeitEstaveisMs) {
        _leitEstaveisOk++; _tLeitEst = now;
        snprintf(buf, sizeof(buf), "Leitura estavel %d/%d", _leitEstaveisOk, nLeitEstaveis);
        logf("INFO", buf);
        if (_leitEstaveisOk >= nLeitEstaveis) _estado = EstadoProt::ST_REARME;
      }
      break;

    case EstadoProt::ST_REARME: {         // religa e avalia janela (V11.9 L656-687)
      contactor(true);
      _rearmeCount++; cntRearmes++;
      if (_rearmeCount == 1) _tJanela = now;
      snprintf(buf, sizeof(buf), "Rearme OK (%d/%d na janela)", _rearmeCount, maxRearmes);
      logf("OK", buf);

      if (_rearmeCount >= maxRearmes) {   // 3 anomalias em 15min → lockout
        contactor(false);                 // R-19: abre
        if (_lockoutNivel < 3) _lockoutNivel++;
        _estado = EstadoProt::ST_LOCKOUT;
        _tLockout = now; cntLockouts++; _tTesteRede = now;
        snprintf(buf, sizeof(buf), "LOCKOUT nivel %d (%lumin) — %d anomalias em %lumin (religamento progressivo — pratica de engenharia, ver NORMS-001)",
                 _lockoutNivel, (unsigned long)(tLockoutMs[_lockoutNivel] / 60000),
                 maxRearmes, (unsigned long)(tJanelaMs / 60000));
        logf("ERRO", buf);
      } else {
        _estado = EstadoProt::ST_OK;
      }
      break;
    }

    case EstadoProt::ST_LOCKOUT: {        // progressivo, teste periódico (V11.9 L689-719)
      uint32_t dur = tLockoutMs[_lockoutNivel < 4 ? _lockoutNivel : 3];
      bool timerExpirou = (now - _tLockout) >= dur;

      if ((now - _tTesteRede) >= tTesteLockMs) {   // testa rede a cada 30s
        _tTesteRede = now;
        if (estavel) {                    // antecipa saída se estável
          logf("INFO", "Rede estavel durante lockout — antecipando rearme");
          _rearmeCount = 0; _leitEstaveisOk = 0;
          _tDesarme = now; _estado = EstadoProt::ST_ESTABIL;
          break;
        }
        snprintf(buf, sizeof(buf), "Lockout N%d — rede ainda instavel (teste %lus)",
                 _lockoutNivel, (unsigned long)((now - _tLockout) / 1000));
        logf("WARN", buf);
      }
      if (timerExpirou) {
        logf("INFO", "Timer de lockout expirado — verificando rede");
        _rearmeCount = 0; _leitEstaveisOk = 0;
        _tDesarme = now; _estado = EstadoProt::ST_ESTABIL;
      }
      break;
    }

    case EstadoProt::ST_MANUAL:
      break;                             // operador no controle — não faz nada
  }
}
}
