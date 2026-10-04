#pragma once
#include <Arduino.h>
#include "Types.h"

// ═══════════════════════════════════════════════════════════════════════════
// PZEM-004T-100A V4.0 — wrapper (CHG-20260906-003)
// Porte do uso real do V11.9 (lib PZEM004Tv30, UART1 addr 0x01 / UART2 0x02,
// L390-401: lerPZEM + classificarTensao L382-387). O 3º canal fica condicionado
// ao ADR-002 (UART do PZEM-3) — nada de pinagem hardcode aqui (R-10).
// Pinagem canônica: hardware/PINOUT/PINOUT-S3-CANONICAL.md
// ═══════════════════════════════════════════════════════════════════════════
#include <PZEM004Tv30.h>

namespace proteve {

// Faixas de classificação — portadas do V11.9 (classificarTensao, L382-387).
// R-14: faixas são parâmetros históricos/configuráveis, não leis.
struct VoltageRange {
  float vNom = 220.0f, vMin = 187.0f, vMax = 242.0f, vFalta = 50.0f;
};

struct PzemReading {
  float v = 0, i = 0, p = 0, e = 0, hz = 0, pf = 0;
  bool ok = false;
  ClassTensao cl = ClassTensao::CL_FALTA;
};

class PzemDriver {
public:
  bool begin(HardwareSerial& serial, uint8_t address, int rxPin, int txPin);
  PzemReading readAll();          // NaN-safe (porte do lerPZEM V11.9 L392-401)
  bool resetEnergy();
  // classificarTensao (V11.9 L382-387): CRITICA se <90% Vmin ou >107% Vmax
  static ClassTensao classify(float v, const VoltageRange& r);
  // redeDefeituosa / redeEstavelAgora (V11.9 L414-431):
  // defeituosa = nenhum canal OK, ou algum canal CRITICA/FALTA
  static bool redeDefeituosa(const PzemReading& a, const PzemReading& b);
  static bool redeEstavel(const PzemReading& a, const PzemReading& b);

private:
  PZEM004Tv30* _pzem = nullptr;
  VoltageRange _range;
};
}
