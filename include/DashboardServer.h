#pragma once
#include "Types.h"
namespace proteve {
class Storage; class AssetManager; class TelemetryEngine; class AlertManager; class OIE;
// Dashboard Web (herdado do V11.9) evoluindo para interface operacional (§38):
// visão geral, equipamentos, DNA, energia, qualidade, saúde, risco, alertas ativos,
// eventos, manutenção, relatórios, IA/chat, configuração, administração.
// Toda lógica vem do OIE — o dashboard não tem lógica própria.
class DashboardServer {
public:
  void begin(Storage& s, AssetManager& a, TelemetryEngine& t, AlertManager& al, OIE& o);
  void tick(uint32_t now);
  // API JSON (base já existia no V11.9)
};
}
