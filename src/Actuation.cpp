// Actuation.cpp — CHG-20260906-015 (R-09/R-18/R-19 + ADR-003 ALARM-ONLY)
// Fluxo obrigatório: SAFETY POLICY → AUTHORIZATION → PERMISSION → CONFIRMATION
// → EXECUTION. Comando sem confirmação física NÃO é "carga desligada" (R-09).
// MVP (ADR-003): a ÚNICA saída autorizada é a contatora de PROTEÇÃO da FSM
// ("KPROT") — o compressor nunca é cortado por software no piloto. GPIO de saída
// fica DESABILITADO (-1) até a validação física da pinagem (R-10): na bancada,
// nada energiza nada sem o PINOUT confirmado.

#include "Actuation.h"
#include "EventEngine.h"
#include "AlertManager.h"
#include "Storage.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {

// Pino de saída: -1 = desabilitado até validação da pinagem na bancada (R-10).
// Quando o PINOUT canônico validar o relé, este valor vem de Config/NVS.
static int RELAY_PIN = -1;

void Actuation::begin(Storage& s, EventEngine& ev, AlertManager& al) {
  _storage = &s; _events = &ev; _alerts = &al;
  for (uint8_t i = 0; i < MAX_PENDING; i++) _pending[i].active = false;
  if (RELAY_PIN >= 0) {
    pinMode(RELAY_PIN, OUTPUT);
    // R-19 fail-safe: saída em estado CONHECIDO — contatora NA = desenergizada
    digitalWrite(RELAY_PIN, LOW);
  }
}

bool Actuation::safetyPolicyAllows(Command cmd, const char* assetId) {
  (void)cmd;
  if (assetId == nullptr) return false;
  // MVP ALARM-ONLY (ADR-003): só a contatora de proteção da FSM pode ser
  // comandada. Qualquer outra carga (compressor, EV, cozinha…) é recusada
  // até existir política autorizada + feedback físico validado em campo.
  return (strncmp(assetId, "KPROT", 5) == 0);
}

ActuationFeedback Actuation::command(const char* assetId, Command cmd, Authorization auth,
                                     const char* policyRef) {
  if (!safetyPolicyAllows(cmd, assetId)) {
    if (_events) {
      char payload[96];
      snprintf(payload, sizeof(payload),
               "{\"refused\":\"ALARM-ONLY\",\"asset\":\"%s\"}", assetId ? assetId : "?");
      _events->emit(EventType::ACTUATION, Severity::WARNING, "ACTUATION", assetId, payload);
    }
    return ActuationFeedback::FAILED;
  }
  // A contatora de proteção só aceita política pré-autorizada (a FSM do V11.9)
  // ou usuário humano. OIE nunca atua sozinho (R-09 + ADR-004 Policy Engine).
  if (auth == Authorization::OIE_RECOMMENDATION_ONLY) return ActuationFeedback::FAILED;

  // EXECUÇÃO — GPIO desabilitado até pinagem validada: registramos a INTENÇÃO
  // e o estado pendente de feedback. Na bancada com relé validado, o digitalWrite
  // entra aqui (R-10: pino vem do PINOUT canônico, não hardcoded).
  if (RELAY_PIN >= 0) digitalWrite(RELAY_PIN, (cmd == Command::CONTACTOR_ON) ? HIGH : LOW);

  // Registra PENDING até confirmação física (contato auxiliar NO/NC — §43)
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    if (_pending[i].active) continue;
    _pending[i].active = true;
    snprintf(_pending[i].assetId, sizeof(_pending[i].assetId), "%s", assetId);
    _pending[i].cmd = cmd;
    _pending[i].sentTs = millis();
    break;
  }
  if (_events) {
    char payload[96];
    snprintf(payload, sizeof(payload), "{\"cmd\":\"%s\",\"auth\":%d,\"policy\":\"%s\"}",
             cmd == Command::CONTACTOR_ON ? "ON" : "OFF", (int)auth,
             policyRef ? policyRef : "-");
    _events->emit(EventType::ACTUATION, Severity::NOTICE, "ACTUATION", assetId, payload);
  }
  return ActuationFeedback::PENDING;
}

void Actuation::tick(uint32_t now) {
  // R-09/§43: comando sem feedback físico não pode ser registrado como executado.
  // Sem contato auxiliar no MVP: PENDING > 5s → TIMEOUT_UNCONFIRMED + evento
  // ("comando enviado, estado físico não confirmado" — RECONSTRUCTION §43).
  // TODO(P0.0-bancada): ler o contato auxiliar real aqui e marcar CONFIRMED.
  for (uint8_t i = 0; i < MAX_PENDING; i++) {
    if (!_pending[i].active) continue;
    if ((now - _pending[i].sentTs) > FEEDBACK_TIMEOUT_MS) {
      _pending[i].active = false;
      if (_events) {
        _events->emit(EventType::COMMAND_UNCONFIRMED, Severity::WARNING,
                      "ACTUATION", _pending[i].assetId);
      }
    }
  }
}

bool Actuation::isConfirmed(const char* assetId) const {
  (void)assetId;
  // Sem hardware de feedback ainda — sempre falso (honestidade R-01).
  return false;
}
}
