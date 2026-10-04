#pragma once
#include "Types.h"
#include "Storage.h"
namespace proteve {
class EventEngine; class AlertManager;
// Atuação (R-09): SAFETY POLICY → AUTHORIZATION → PERMISSION → CONFIRMATION → EXECUTION.
// Comando OFF exige FEEDBACK OFF via contato auxiliar NO/NC. Sem feedback no timeout:
// "⚠️ Comando enviado, mas estado físico não confirmado" + evento COMMAND_UNCONFIRMED.
// R-18 (PEMB-018/019/020): reset remoto NUNCA rearma disjuntor de potência — só
// destrava lockout/contatoras de carga não-crítica. Disjuntor de potência é SEMPRE manual.
// R-19: fail-safe elétrico — todo circuito de acionamento é normalmente aberto (NA) do
// lado de segurança; perda de alimentação de controle nunca resulta em carga energizada
// por omissão. Nenhum caminho de código deste header pode violar R-18/R-19.
class Actuation {
public:
  void begin(Storage& s, EventEngine& ev, AlertManager& al);
  ActuationFeedback command(const char* assetId, Command cmd, Authorization auth,
                            const char* policyRef = nullptr);
  void tick(uint32_t now);              // aguarda confirmação física dos comandos PENDING
  bool isConfirmed(const char* assetId) const;
private:
  bool safetyPolicyAllows(Command cmd, const char* assetId); // proteção física é intocável
  struct Pending { char assetId[12]; Command cmd; uint32_t sentTs; bool active = false; };
  static constexpr uint8_t MAX_PENDING = 4;
  Pending _pending[MAX_PENDING];
  static constexpr uint32_t FEEDBACK_TIMEOUT_MS = 5000;  // R-09: sem contato auxiliar = NÃO confirmado
  Storage* _storage = nullptr; EventEngine* _events = nullptr; AlertManager* _alerts = nullptr;
};
}
