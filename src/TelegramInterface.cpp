// TelegramInterface.cpp — CHG-20260906-023 — STUB HONESTO (P1)
// Interface operacional (RECONSTRUCTION §37): consulta "como está o compressor?",
// alertas com lifecycle (não dilúvio — AUDIT-TG-001), comando com permissão.
// Telegram 2.0 "silêncio inteligente" (ADR-004): INFO/SUMMARY/ATTENTION/RISK/
// CRITICAL + dedup + cooldown. Credenciais: portal cativo→NVS (R-05), JAMAIS
// hardcoded — lição registrada da auditoria de segurança do Drive.

#include "TelegramInterface.h"
#include "OIE.h"
#include "AlertManager.h"
#include "AssetManager.h"
#include <Arduino.h>

namespace proteve {
void TelegramInterface::begin(OIE& o, AlertManager& al, AssetManager& am) {
  (void)o; (void)al; (void)am;
  // TODO(P1): token via NVS;long polling UniversalBot; raio-X multi-canal (AUDIT-TG-001)
}
void TelegramInterface::tick(uint32_t now) { (void)now; }
}
