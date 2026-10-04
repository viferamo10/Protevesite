// AssetManager.cpp — CHG-20260906-013 (P0.2 — Asset DNA / RECONSTRUCTION §10, §20, §67)
// Cada equipamento ganha ID permanente AST-000001… + campos de política LMDC.
// MVP: registro em RAM + persistência JSON no Storage (config versionada).
// O DNA operacional acumulado (horas, consumo histórico, degradação) vive em
// telemetry/events/maintenance — aqui só o CADASTRO (nunca duplicar dado derivado).

#include "AssetManager.h"
#include "Storage.h"
#include "EventEngine.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {

void AssetManager::begin(Storage& storage) {
  _storage = &storage;
  _count = 0;
  load();
  // Ativo do próprio MVP: a contatora de proteção da FSM (V11.9) — a única
  // saída autorizada a agir no MVP (ADR-003: ALARM-ONLY para todo o resto).
  if (byId("KPROT") == nullptr) {
    const char* id = createAsset("Contatora de Protecao (FSM)", "ACTUATOR", "QDC-comando");
    (void)id;
  }
}

bool AssetManager::load() {
  if (_storage == nullptr) return false;
  char buf[1024];
  if (!_storage->readConfig(buf, sizeof(buf))) return false;
  // Formato: {"assets":[{"id":"AST-000001","name":"...","type":"...","loc":"...","crit":1,"shed":0}]}
  _count = 0;
  char* p = buf;
  while ((p = strstr(p, "\"id\":\"AST-")) != nullptr && _count < MAX_ASSETS) {
    AssetRec& a = _assets[_count];
    memset(&a, 0, sizeof(a));
    sscanf(p, "\"id\":\"%11[^\"]\",", a.id);
    const char* q = strstr(p, "\"name\":\"");
    if (q) sscanf(q, "\"name\":\"%31[^\"]\",", a.name);
    q = strstr(p, "\"type\":\"");
    if (q) sscanf(q, "\"type\":\"%15[^\"]\",", a.type);
    q = strstr(p, "\"loc\":\"");
    if (q) sscanf(q, "\"loc\":\"%23[^\"]\",", a.location);
    q = strstr(p, "\"crit\":");
    if (q) { int c = 3; sscanf(q, "\"crit\":%d,", &c); a.criticality = (Criticality)c; }
    q = strstr(p, "\"shed\":");
    if (q) { int sh = 0; sscanf(q, "\"shed\":%d,", &sh); a.sheddingAllowed = (sh != 0); }
    q = strstr(p, "\"auto\":");
    if (q) { int au = 0; sscanf(q, "\"auto\":%d,", &au); a.automaticControlAllowed = (au != 0); }
    // valida id
    if (a.id[0] == 'A') { _count++; }
    p += 10;
  }
  return (_count > 0);
}

bool AssetManager::save() {
  if (_storage == nullptr) return false;
  char buf[1024];
  int off = snprintf(buf, sizeof(buf), "{\"assets\":[");
  for (uint8_t i = 0; i < _count && off < (int)sizeof(buf) - 96; i++) {
    const AssetRec& a = _assets[i];
    off += snprintf(buf + off, sizeof(buf) - off,
      "%s{\"id\":\"%s\",\"name\":\"%s\",\"type\":\"%s\",\"loc\":\"%s\",\"crit\":%d,\"shed\":%d,\"auto\":%d}",
      (i ? "," : ""), a.id, a.name, a.type, a.location,
      (int)a.criticality, (int)a.sheddingAllowed, (int)a.automaticControlAllowed);
  }
  snprintf(buf + off, sizeof(buf) - off, "]}");
  return _storage->writeConfig(buf);
}

const char* AssetManager::createAsset(const char* name, const char* type, const char* location) {
  if (_count >= MAX_ASSETS) return nullptr;
  AssetRec& a = _assets[_count];
  memset(&a, 0, sizeof(a));
  snprintf(a.id, sizeof(a.id), "AST-%06u", (unsigned)(_count + 1));
  snprintf(a.name, sizeof(a.name), "%s", name ? name : "?");
  snprintf(a.type, sizeof(a.type), "%s", type ? type : "LOAD");
  snprintf(a.location, sizeof(a.location), "%s", location ? location : "-");
  a.criticality = Criticality::CT_MED;
  a.sheddingAllowed = false;             // §20: default conservador
  a.automaticControlAllowed = false;     // ADR-003: MVP ALARM-ONLY
  a.operationalPriority = 3;
  const char* id = a.id;
  _count++;
  save();                                 // R-08: cadastro persiste na hora
  return id;
}

const char* AssetManager::byId(const char* id) const {
  if (!id) return nullptr;
  for (uint8_t i = 0; i < _count; i++) {
    if (strncmp(_assets[i].id, id, sizeof(_assets[i].id)) == 0) return _assets[i].name;
  }
  return nullptr;
}

void AssetManager::applyHumanCorrection(const char* inferredAssetId, const char* realAssetId,
                                        const char* evidenceJson) {
  // RECONSTRUCTION §9: a correção humana vira CONHECIMENTO — registra a associação
  // assinatura→ativo real com a evidência. No MVP: evento + TODO(P1) alimentar o
  // modelo NILM (LearningEngine::updateEquipmentRecord pelo índice do match).
  (void)inferredAssetId; (void)realAssetId; (void)evidenceJson;
  // events->emit(CONFIG_CHANGE, NOTICE, "ASSET-DNA", realAssetId, evidenceJson) — P1
  // (quando AssetManager tiver EventEngine na composição do kernel).
}
}
