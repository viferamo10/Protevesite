// TelemetryEngine.cpp — CHG-20260906-011 (contém CHG-006: tarifa/bandeira ANEEL)
// Coleta (SensorManager), timestamp RTC, agregação, anti-dupla-contagem (R-11),
// integração com LearningEngine (baseline 30s + anomalia + energia/custo) e
// CORREÇÃO DO LEGACY-BUG-001 (dayRollover explícito na virada de dia).
// Origem: loop de coleta do V11.9 (L586-620 lmUpdate + L602-604 energia) e
// tarifas/bandeiras (V11.9 L100-105): base 0,75 R$/kWh + acrescimos por bandeira.

#include "TelemetryEngine.h"
#include "SensorManager.h"
#include "LearningEngine.h"
#include "Storage.h"
#include "Types.h"
#include <Arduino.h>
#include <stdio.h>

namespace proteve {

void TelemetryEngine::begin(Storage& s, SensorManager& sm, AssetManager& am) {
  _storage = &s;
  _sensors = &sm;
  _assets = &am;
  _lastTickMs = 0;
  _lastLmMs = 0;
  _lastDayHour = 255;
}

void TelemetryEngine::setLearningEngine(LearningEngine* le) {
  _learning = le;
}

float TelemetryEngine::effectiveTariff() const {
  // CHG-20260906-006 — V11.9 L100-105: tarifa base 0,75 + acréscimo da bandeira vigente.
  // TODO(P1): bandeira automática via config (hoje fixa, default Verde).
  if (_bandeira > 3) return TARIFA_BASE_HISTORICA;
  return TARIFA_BASE_HISTORICA + ACRESCIMO_BAN[_bandeira];
}

void TelemetryEngine::setBandeira(uint8_t b) {
  if (b > 3) b = 0;
  _bandeira = b;
}

bool TelemetryEngine::latest(uint8_t channel, Snapshot& out) const {
  if (_sensors == nullptr) return false;
  return _sensors->getSnapshot(channel, out);
}

float TelemetryEngine::totalLoadW() const {
  // R-11 — hierarquia de medidores: no MVP da câmara fria os canais são circuitos
  // INDEPENDENTES (compressor | geral), sem medidor pai, então a soma é legítima.
  // QUANDO houver medidor pai (residencial: geral + filhos), somar SÓ o pai —
  // nunca pai+filhos (dupla contagem). TODO(P1): hierarquia no AssetManager.
  Snapshot s{};
  float total = 0;
  for (uint8_t ch = 0; ch < 2; ch++) {
    if (_sensors != nullptr && _sensors->getSnapshot(ch, s) && s.ok) {
      total += s.w;
    }
  }
  return total;
}

void TelemetryEngine::tick(uint32_t now) {
  // --- Ciclo de telemetria (T_LEITURA_MS 1500ms do V11.9) ---
  if (now - _lastTickMs < 1500) return;
  const uint32_t dtMs = (_lastTickMs == 0) ? 1500 : (now - _lastTickMs);
  _lastTickMs = now;

  // Agregação dos canais válidos
  Snapshot s0{}, s1{};
  const bool ok0 = _sensors->getSnapshot(0, s0);
  const bool ok1 = _sensors->getSnapshot(1, s1);
  const float pTot = (ok0 && s0.ok ? s0.w : 0) + (ok1 && s1.ok ? s1.w : 0);
  const float iTot = (ok0 && s0.ok ? s0.a : 0) + (ok1 && s1.ok ? s1.a : 0);
  const float pfAvg = (ok0 && ok1 && s0.ok && s1.ok && s1.pf > 0.01f)
                          ? (s0.pf + s1.pf) / 2.0f
                          : ((ok0 && s0.ok) ? s0.pf : ((ok1 && s1.ok) ? s1.pf : 0.0f));

  // Persistência JSONL (amostra agregada; origem REAL por enquanto — R-06)
  char line[160];
  snprintf(line, sizeof(line),
           "{\"ts\":%lu,\"w\":%.1f,\"a\":%.2f,\"pf\":%.3f,\"w0\":%.1f,\"w1\":%.1f,\"door\":%d,\"t\":%.1f}",
           (unsigned long)(ok0 && s0.ts > 0 ? s0.ts : now / 1000),
           pTot, iTot, pfAvg,
           (ok0 && s0.ok ? s0.w : 0), (ok1 && s1.ok ? s1.w : 0),
           (int)(ok0 ? s0.doorOpen : (ok1 ? s1.doorOpen : false)),
           (ok0 ? s0.tempC : (ok1 ? s1.tempC : -127.0f)));
  if (_storage != nullptr) _storage->appendTelemetry(line);

  // --- Learning (T_LM_UPDATE_MS 30s do V11.9 L586) ---
  if (now - _lastLmMs >= 30000) {
    const uint32_t dtLmMs = (_lastLmMs == 0) ? 30000 : (now - _lastLmMs);
    _lastLmMs = now;

    // Hora do dia: epoch do RTC quando presente; sem RTC não há "hora" confiável
    // (o V11.9 usava getHora do RTC; sem RTC o baseline por hora não deve treinar —
    // senão aprende lixo. R-06: dado ausente degrada confiança, não fabrica valor).
    const uint32_t ts = (ok0 && s0.ts > 0) ? s0.ts : ((ok1 && s1.ts > 0) ? s1.ts : 0);
    if (ts > 1700000000UL && _learning != nullptr) {  // epoch plausível = RTC presente
      const uint8_t hour = (uint8_t)((ts / 3600UL) % 24UL);

      _learning->updateBaseline(hour, pTot, iTot, pfAvg);
      _learning->addEnergy(pTot, (float)dtLmMs / 1000.0f, effectiveTariff(), 0 /*TODO período CHG-012*/);
      _learning->checkAnomaly(hour, pTot);

      // CORREÇÃO DO LEGACY-BUG-001 — rollover diário EXPLÍCITO na virada da meia-noite.
      // No V11.9 este bloco estava dentro do relatório 8h/20h e era inalcançável (h==0
      // nunca ocorria ali). Aqui: uma única chamada na primeira passagem após 23h.
      if (_lastDayHour != 255 && _lastDayHour == 23 && hour == 0) {
        _learning->dayRollover();
      }
      _lastDayHour = hour;
    }
  }
}
}
