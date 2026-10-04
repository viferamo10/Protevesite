// DashboardServer.cpp — CHG-20260906-022 — STUB HONESTO (P1)
// Dashboard consulta o MESMO estado (Command Center — ADR-004): visão geral,
// equipamentos, DNA, energia, qualidade, saúde, risco, alertas, eventos,
// manutenção, relatórios (RECONSTRUCTION §38). WiFi credenciais NUNCA no código
// (R-05) — provisionamento FRIO (portal cativo→NVS) quando entrar.

#include "DashboardServer.h"
#include "Storage.h"
#include "AssetManager.h"
#include "TelemetryEngine.h"
#include "AlertManager.h"
#include "OIE.h"
#include <Arduino.h>

namespace proteve {
void DashboardServer::begin(Storage& s, AssetManager& a, TelemetryEngine& t,
                            AlertManager& al, OIE& o) {
  (void)s; (void)a; (void)t; (void)al; (void)o;
  // TODO(P1): WiFi (NVS) + AsyncWebServer + JSON API (mesmos dados do V11.9)
}
void DashboardServer::tick(uint32_t now) { (void)now; }
}
