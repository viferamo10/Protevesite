#pragma once
#include "Types.h"
namespace proteve {
class OIE; class AlertManager; class AssetManager;
// Telegram (base V11.9) evoluindo de "canal de alertas" para INTERFACE OPERACIONAL (§37):
// "Como está o compressor?" · "Quanto gastei ontem?" · "Quais alarmes ativos?" ·
// futuramente "Coloque o AC do quarto 3 como prioridade baixa."
// Token/chat-ids via NVS/.env (R-05). Toda interpretação passa pelo OIE + políticas.
class TelegramInterface {
public:
  void begin(OIE& o, AlertManager& al, AssetManager& am);
  void tick(uint32_t now);
  // Alertas ativos são entregues como ESTADO (podem lembrar/escalar — R-07)
};
}
