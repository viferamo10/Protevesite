#pragma once
#include <Arduino.h>
namespace proteve {

// ═══════════════════════════════════════════════════════════════════════════
// FW-S3-MVP-001 — Tipos comuns
// CHG-20260906-001 (ver engineering/DECISIONS/EXTRACT-LOG.md)
// ═══════════════════════════════════════════════════════════════════════════

// --- Qualidade de dados (R-06) ---
enum class DataOrigin   : uint8_t { REAL, SIMULATED, DERIVED };
enum class SensorState  : uint8_t { ONLINE, OFFLINE, SIMULATED, STALE, INVALID };

// --- Eventos (R-08): tudo vira evento, append-only ---
enum class EventType : uint16_t {
  BOOT, SENSOR_ONLINE, SENSOR_OFFLINE, VOLTAGE_LOW, VOLTAGE_HIGH, CURRENT_HIGH,
  TEMP_HIGH, DOOR_OPEN, DOOR_CLOSED, COMPRESSOR_START, COMPRESSOR_STOP,
  ALERT_OPEN, ALERT_ESCALATED, ALERT_RESOLVED, MAINTENANCE, CONFIG_CHANGE,
  LOGIN, TELEGRAM, ACTUATION, COMMAND_UNCONFIRMED, FSM_STATE
};
enum class Severity : uint8_t { INFO, NOTICE, WARNING, CRITICAL };

// --- Alert Lifecycle (R-07): alerta é estado, não mensagem ---
enum class AlertStatus : uint8_t { OPEN, ACKNOWLEDGED, ACTIVE, ESCALATED, RESOLVED, CLOSED };

// --- Atuação (R-09/R-18/R-19): autorização + confirmação física + fail-safe ---
enum class ActuationFeedback : uint8_t { PENDING, CONFIRMED, FAILED, TIMEOUT_UNCONFIRMED };
enum class Command      : uint8_t { CONTACTOR_OFF, CONTACTOR_ON };
enum class Authorization: uint8_t { USER, POLICY_PREAUTHORIZED, OIE_RECOMMENDATION_ONLY };
enum class LmdcMode     : uint8_t { MONITOR, ASSISTED, AUTOMATIC };
enum class Criticality  : uint8_t { CT_CRIT, CT_HIGH, CT_MED, CT_LOW };

// --- Proteção elétrica (portado do V11.9 — CHG-20260906-002) ---
enum class EstadoProt : uint8_t {
  ST_OK = 0, ST_DESARME = 1, ST_ESTABIL = 2, ST_REARME = 3,
  ST_LOCKOUT = 4, ST_MANUAL = 5
};
enum class ClassTensao : uint8_t { CL_ADEQUADA = 0, CL_PRECARIA = 1, CL_CRITICA = 2, CL_FALTA = 3 };
enum class TipoAnomalia : uint8_t { AN_NENHUMA = 0, AN_SOBRETENSAO, AN_SUBTENSAO,
                                    AN_FALTA, AN_CONSUMO_ALTO, AN_EVENTO_USO };

// --- Snapshot de telemetria entregue às engines ---
struct Snapshot {
  uint32_t ts;
  bool ok;                      // canal válido (V11.9: !isnan(v) && hz>0.1)
  float v, a, w, pf, hz, kwh;
  float tempC;                   // DS18B20 (câmara / painel quando disponível)
  bool   doorOpen;
  DataOrigin origin;
};

const char* toString(EventType t);
const char* toString(AlertStatus s);
const char* toString(SensorState s);
const char* toString(DataOrigin o);
const char* toString(EstadoProt e);
const char* toString(ClassTensao c);
const char* toString(TipoAnomalia a);
}
