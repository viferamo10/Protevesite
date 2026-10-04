/**
 * PROTEVE — FW-S3-MVP-001
 * main.cpp — SOMENTE inicialização e orquestração.
 *
 * REGRA (R-03/R-07 das Rules of Continuity): nenhuma lógica de negócio aqui.
 * Este arquivo sobe o kernel na ordem de dependências e despacha ticks.
 *
 * Kernel completo após CHG-20260906-001..023:
 *   Núcleo: Types · ProtectionEngine(FSM) · PzemDriver · SensorManager · Storage
 *           EventEngine · TelemetryEngine(+tarifa CHG-006/LEGACY-BUG-001) ·
 *           LearningEngine · NilmDatabase · AssetManager · AlertManager · Actuation
 *   Stubs P1 (ligam o link, lógica na fila): OIE · Rule/Knowledge · LMDC ·
 *           Maintenance · Simulator · Dashboard · Telegram
 */

#include <Arduino.h>

#include "Config.h"
#include "Types.h"
#include "PzemDriver.h"
#include "ProtectionEngine.h"
#include "NilmDatabase.h"
#include "LearningEngine.h"
#include "Storage.h"
#include "AssetManager.h"
#include "SensorManager.h"
#include "TelemetryEngine.h"
#include "EventEngine.h"
#include "RuleEngine.h"
#include "KnowledgeEngine.h"
#include "AlertManager.h"
#include "OIE.h"
#include "Actuation.h"
#include "LMDC.h"
#include "Maintenance.h"
#include "Simulator.h"
#include "DashboardServer.h"
#include "TelegramInterface.h"

using namespace proteve;

// --- Kernel (ordem de dependência) ---
Storage           storage;
AssetManager      assets;         // P0.2 — AST-000001… + políticas LMDC
SensorManager     sensors;
TelemetryEngine   telemetry;      // dona do ciclo LM (30s, gate RTC) + tarifa
EventEngine       events;         // P0.3 — tudo vira evento (R-08)
RuleEngine        rules;
KnowledgeEngine   knowledge;
LearningEngine    learning;       // CHG-005 (V11.9, dois alphas)
ProtectionEngine protection;     // CHG-002 (FSM V11.9)
AlertManager      alerts;        // P0.4 — lifecycle (R-07)
OIE               oie;
Actuation         actuation;     // R-09/R-18/R-19 — ALARM-ONLY (ADR-003)
LMDC              lmdc;          // MONITOR até validação (ADR-003)
Maintenance       maintenance;
Simulator         simulator;
DashboardServer   dashboard;
TelegramInterface telegram;

uint32_t lastTickMs = 0;

// --- Callbacks da FSM (CHG-002: hardware desacoplado; R-10: pinagem no PINOUT) ---
static void contactorCallback(bool energize) {
  // A contatora aqui é a de PROTEÇÃO da linha de comando (V11.9) — a única
  // saída autorizada do MVP (ADR-003). Feedback físico: Actuation emite
  // COMMAND_UNCONFIRMED se o contato auxiliar não confirmar (R-09/§43).
  actuation.command("KPROT",
                    energize ? Command::CONTACTOR_ON : Command::CONTACTOR_OFF,
                    Authorization::POLICY_PREAUTHORIZED,
                    "FSM-V11.9-lockout");
}
static Severity sevFromLevel(const char* level) {
  if (level == nullptr) return Severity::INFO;
  if (strcmp(level, "CRITICAL") == 0) return Severity::CRITICAL;
  if (strcmp(level, "WARNING") == 0)  return Severity::WARNING;
  if (strcmp(level, "NOTICE") == 0)   return Severity::NOTICE;
  return Severity::INFO;
}
static void fsmLogCallback(const char* level, const char* msg) {
  // FSM também vira EVENTO (R-08): transitions desarme/estabil/rearme/lockout
  // ficam no histórico operacional — o que o V11.9 só mandava ao Telegram.
  Serial.printf("[FSM][%s] %s\n", level, msg);
  events.emit(EventType::FSM_STATE, sevFromLevel(level), "FSM", nullptr, msg);
}

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.printf("\n[PROTEVE] %s v%s — boot\n", FW_ID, FW_VERSION);

  // 1) Persistência local-first (SD HW-125 + fallback LittleFS, não-fatal)
  storage.begin();
  // 2) Asset DNA (P0.2): cadastro AST-* + KPROT (contatora de proteção)
  assets.begin(storage);
  // 3) Sensores — PZEM×2 (pinagem validada em campo no V11.9: 26/27 e 16/17),
  //    DS18B20, porta, DS3231 (R-10: no S3 tudo se revalida na bancada)
  sensors.begin(storage);
  // 4) Telemetria (hierarquia anti dupla contagem R-11; ciclo LM 30s com gate RTC)
  telemetry.begin(storage, sensors, assets);
  telemetry.setLearningEngine(&learning);
  // 5) Eventos (R-08)
  events.begin(storage);
  events.emit(EventType::BOOT, Severity::INFO, "KERNEL");
  // 6) Inteligência
  rules.begin(storage);
  knowledge.begin(storage);
  learning.begin();                        // CHG-005: parâmetros V11.9 = defaults
  alerts.begin(storage, events);           // P0.4 — lifecycle R-07
  protection.begin(contactorCallback, fsmLogCallback);
  // 7) OIE — recomenda, nunca atua fora de política (R-09)
  oie.begin(rules, knowledge, learning, alerts, assets, telemetry);
  // 8) Operações
  actuation.begin(storage, events, alerts);
  lmdc.begin(assets, actuation, alerts);   // MONITOR (ADR-003)
  maintenance.begin(storage, events, learning);
  // 9) Interfaces (P1 — stubs)
  dashboard.begin(storage, assets, telemetry, alerts, oie);
  telegram.begin(oie, alerts, assets);     // credenciais NVS — R-05
  // 10) Simulador — dados SIMULATED nunca misturados com REAL (R-06)
  simulator.begin(sensors, telemetry);
  simulator.setEnabled(false);

  Serial.println("[PROTEVE] kernel up — núcleo P0 completo (EXTRACT-LOG CHG-001..023).");
}

void loop() {
  const uint32_t now = millis();

  // Ciclo de aquisição (V11.9: T_LEITURA_MS = 1500 ms)
  if (now - lastTickMs >= 1500) {
    lastTickMs = now;

    sensors.tick(now);        // estados ONLINE/STALE/OFFLINE + eventos de sensor
    telemetry.tick(now);      // coleta + timestamp RTC + LM 30s + dayRollover (§51)

    // FSM de proteção (CHG-002) — predicados dos canais ativos (V11.9 L414-431)
    Snapshot s1{}, s2{};
    const bool has1 = telemetry.latest(0, s1), has2 = telemetry.latest(1, s2);
    const bool def = (!has1 && !has2) ||
        (has1 && (PzemDriver::classify(s1.v, VoltageRange{}) >= ClassTensao::CL_CRITICA)) ||
        (has2 && (PzemDriver::classify(s2.v, VoltageRange{}) >= ClassTensao::CL_CRITICA));
    const bool est = (has1 || has2) && !def;
    protection.tick(now, def, est);

    // Snapshot de referência p/ engines (canal 0, senão canal 1)
    const Snapshot& ref = has1 ? s1 : s2;

    rules.tick(now, ref);     // regras (P1)
    oie.tick(now);            // DETECTAR→…→RECOMENDAR (P1)
    alerts.tick(now);         // lifecycle: escalação de não-reconhecidos (P0.4)
    lmdc.tick(now);           // MONITOR (P1)
    actuation.tick(now);      // feedback físico: timeout → COMMAND_UNCONFIRMED
    maintenance.tick(now);
    dashboard.tick(now);
    telegram.tick(now);
    simulator.tick(now);
  }
}
