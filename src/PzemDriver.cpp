// PzemDriver.cpp — CHG-20260906-007 (PzemDriver implementation)
// Wrapper para PZEM-004T-100A V4.0 (lib mandulaj/PZEM-004T-100A).
// Origem: V11.9 L382-387 (classificarTensao), L392-401 (lerPZEM NaN-safe),
// L414-431 (redeEstavelAgora / redeDefeituosa).
// Pinagem NUNCA hardcoded aqui (R-10): pinos vêm por parâmetro do caller/PINOUT canônico.

#include "PzemDriver.h"
#include <math.h>

namespace proteve {

bool PzemDriver::begin(HardwareSerial& serial, uint8_t address, int rxPin, int txPin) {
  if (_pzem != nullptr) {
    delete _pzem;
    _pzem = nullptr;
  }
  // V11.9 L290-291: instancia PZEM004Tv30 especificando serial, rx, tx e endereço RTU
  _pzem = new PZEM004Tv30(serial, rxPin, txPin, address);
  return (_pzem != nullptr);
}

PzemReading PzemDriver::readAll() {
  PzemReading d;
  if (!_pzem) return d;

  // Porte do lerPZEM V11.9 L392-401: leitura NaN-safe
  d.v  = _pzem->voltage();
  d.i  = _pzem->current();
  d.p  = _pzem->power();
  d.e  = _pzem->energy();
  d.hz = _pzem->frequency();
  d.pf = _pzem->pf();

  // V11.9 L397: ok = !isnan(v) && !isnan(hz) && hz > 0.1f
  d.ok = (!isnan(d.v) && !isnan(d.hz) && d.hz > 0.1f);
  if (!d.ok) {
    d.v = 0.0f;
  }

  if (isnan(d.i))  d.i = 0.0f;
  if (isnan(d.p))  d.p = 0.0f;
  if (isnan(d.e))  d.e = 0.0f;
  if (isnan(d.hz)) d.hz = 0.0f;
  if (isnan(d.pf)) d.pf = 0.0f;

  d.cl = classify(d.v, _range);
  return d;
}

bool PzemDriver::resetEnergy() {
  if (_pzem != nullptr) {
    return _pzem->resetEnergy();
  }
  return false;
}

ClassTensao PzemDriver::classify(float v, const VoltageRange& r) {
  // Porte de classificarTensao V11.9 L382-387
  if (isnan(v) || v < r.vFalta) {
    return ClassTensao::CL_FALTA;
  }
  if (v < r.vMin * 0.90f || v > r.vMax * 1.07f) {
    return ClassTensao::CL_CRITICA;
  }
  if (v < r.vMin || v > r.vMax) {
    return ClassTensao::CL_PRECARIA;
  }
  return ClassTensao::CL_ADEQUADA;
}

bool PzemDriver::redeDefeituosa(const PzemReading& a, const PzemReading& b) {
  // Porte de redeDefeituosa V11.9 L422-428:
  // defeituosa = nenhum canal OK OU algum canal ativo em CRITICA/FALTA
  if (!a.ok && !b.ok) return true;
  if (a.ok && (a.cl == ClassTensao::CL_CRITICA || a.cl == ClassTensao::CL_FALTA)) return true;
  if (b.ok && (b.cl == ClassTensao::CL_CRITICA || b.cl == ClassTensao::CL_FALTA)) return true;
  return false;
}

bool PzemDriver::redeEstavel(const PzemReading& a, const PzemReading& b) {
  // Porte de redeEstavelAgora V11.9 L414-419:
  // estavel = pelo menos 1 canal OK E nenhum canal ativo em CRITICA/FALTA
  if (!a.ok && !b.ok) return false;
  if (a.ok && (a.cl == ClassTensao::CL_CRITICA || a.cl == ClassTensao::CL_FALTA)) return false;
  if (b.ok && (b.cl == ClassTensao::CL_CRITICA || b.cl == ClassTensao::CL_FALTA)) return false;
  return true;
}

}
