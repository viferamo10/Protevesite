#include "Types.h"
namespace proteve {
// CHG-20260906-001 — implementação dos toString (consumo: EventEngine/log/API).
const char* toString(EventType t) {
  switch (t) {
    case EventType::BOOT: return "BOOT"; case EventType::SENSOR_ONLINE: return "SENSOR_ONLINE";
    case EventType::SENSOR_OFFLINE: return "SENSOR_OFFLINE"; case EventType::VOLTAGE_LOW: return "VOLTAGE_LOW";
    case EventType::VOLTAGE_HIGH: return "VOLTAGE_HIGH"; case EventType::CURRENT_HIGH: return "CURRENT_HIGH";
    case EventType::TEMP_HIGH: return "TEMP_HIGH"; case EventType::DOOR_OPEN: return "DOOR_OPEN";
    case EventType::DOOR_CLOSED: return "DOOR_CLOSED"; case EventType::COMPRESSOR_START: return "COMPRESSOR_START";
    case EventType::COMPRESSOR_STOP: return "COMPRESSOR_STOP"; case EventType::ALERT_OPEN: return "ALERT_OPEN";
    case EventType::ALERT_ESCALATED: return "ALERT_ESCALATED"; case EventType::ALERT_RESOLVED: return "ALERT_RESOLVED";
    case EventType::MAINTENANCE: return "MAINTENANCE"; case EventType::CONFIG_CHANGE: return "CONFIG_CHANGE";
    case EventType::LOGIN: return "LOGIN"; case EventType::TELEGRAM: return "TELEGRAM";
    case EventType::ACTUATION: return "ACTUATION"; case EventType::COMMAND_UNCONFIRMED: return "COMMAND_UNCONFIRMED"; case EventType::FSM_STATE: return "FSM_STATE";
  }
  return "?";
}
const char* toString(AlertStatus s) {
  switch (s) { case AlertStatus::OPEN: return "OPEN"; case AlertStatus::ACKNOWLEDGED: return "ACKNOWLEDGED";
    case AlertStatus::ACTIVE: return "ACTIVE"; case AlertStatus::ESCALATED: return "ESCALATED";
    case AlertStatus::RESOLVED: return "RESOLVED"; case AlertStatus::CLOSED: return "CLOSED"; }
  return "?";
}
const char* toString(SensorState s) {
  switch (s) { case SensorState::ONLINE: return "ONLINE"; case SensorState::OFFLINE: return "OFFLINE";
    case SensorState::SIMULATED: return "SIMULATED"; case SensorState::STALE: return "STALE";
    case SensorState::INVALID: return "INVALID"; }
  return "?";
}
const char* toString(DataOrigin o) {
  switch (o) { case DataOrigin::REAL: return "REAL"; case DataOrigin::SIMULATED: return "SIMULATED";
    case DataOrigin::DERIVED: return "DERIVED"; }
  return "?";
}
const char* toString(EstadoProt e) {
  switch (e) { case EstadoProt::ST_OK: return "OK"; case EstadoProt::ST_DESARME: return "DESARME";
    case EstadoProt::ST_ESTABIL: return "ESTABIL"; case EstadoProt::ST_REARME: return "REARME";
    case EstadoProt::ST_LOCKOUT: return "LOCKOUT"; case EstadoProt::ST_MANUAL: return "MANUAL"; }
  return "?";
}
const char* toString(ClassTensao c) {
  switch (c) { case ClassTensao::CL_ADEQUADA: return "ADEQUADA"; case ClassTensao::CL_PRECARIA: return "PRECARIA";
    case ClassTensao::CL_CRITICA: return "CRITICA"; case ClassTensao::CL_FALTA: return "FALTA"; }
  return "?";
}
const char* toString(TipoAnomalia a) {
  switch (a) { case TipoAnomalia::AN_NENHUMA: return "NENHUMA"; case TipoAnomalia::AN_SOBRETENSAO: return "SOBRETENSAO";
    case TipoAnomalia::AN_SUBTENSAO: return "SUBTENSAO"; case TipoAnomalia::AN_FALTA: return "FALTA";
    case TipoAnomalia::AN_CONSUMO_ALTO: return "CONSUMO_ALTO"; case TipoAnomalia::AN_EVENTO_USO: return "EVENTO_USO"; }
  return "?";
}
}
