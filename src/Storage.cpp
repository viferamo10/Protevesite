// Storage.cpp — CHG-20260906-009
// Local-first Storage (R-12): MicroSD (HW-125 via SPI) com fallback LittleFS.
// Gravação append-only JSONL para eventos e telemetria.
// Inicialização NÃO FATAL: falhas no cartão SD logam o erro e continuam em LittleFS/RAM.
// Pinagem canônica SPI2: SCK 12, MISO 13, MOSI 11, CS 10 (PINOUT-S3-CANONICAL).

#include "Storage.h"
#include <Arduino.h>
#include <SPI.h>

namespace proteve {

bool Storage::begin() {
  _sdPresent = false;
  _fsPresent = false;

  // Inicializa barramento SPI com pinagem canônica S3 (SCK=12, MISO=13, MOSI=11, CS=10)
  SPI.begin(12, 13, 11, _sdCsPin);

  // Tentativa não-fatal do MicroSD HW-125
  if (SD.begin(_sdCsPin, SPI, 4000000)) {
    uint8_t cardType = SD.cardType();
    if (cardType != CARD_NONE) {
      _sdPresent = true;
      Serial.println("[STORAGE] MicroSD HW-125 inicializado com sucesso.");
    } else {
      Serial.println("[STORAGE] MicroSD inserido, mas tipo nao reconhecido.");
    }
  } else {
    Serial.println("[STORAGE] MicroSD HW-125 ausente/falhou. Fallback ativado.");
  }

  // Fallback LittleFS na flash interna
  if (LittleFS.begin(true)) {
    _fsPresent = true;
    Serial.println("[STORAGE] LittleFS inicializado como fallback de armazenamento.");
  } else {
    Serial.println("[STORAGE] ERRO: LittleFS falhou ao montar.");
  }

  // Não-fatal: nunca trava a execução mesmo sem SD/FS
  return true;
}

bool Storage::appendEvent(const char* line) {
  if (!line || strlen(line) == 0) return false;

  bool written = false;

  // 1) Grava no SD se presente
  if (_sdPresent) {
    File file = SD.open("/events.jsonl", FILE_APPEND);
    if (file) {
      file.println(line);
      file.flush();
      file.close();
      written = true;
    }
  }

  // 2) Fallback LittleFS se SD indisponível ou falhou
  if (!written && _fsPresent) {
    File file = LittleFS.open("/events.jsonl", FILE_APPEND);
    if (file) {
      file.println(line);
      file.flush();
      file.close();
      written = true;
    }
  }

  return written;
}

bool Storage::appendTelemetry(const char* line) {
  if (!line || strlen(line) == 0) return false;

  bool written = false;

  if (_sdPresent) {
    File file = SD.open("/telemetry.jsonl", FILE_APPEND);
    if (file) {
      file.println(line);
      file.flush();
      file.close();
      written = true;
    }
  }

  if (!written && _fsPresent) {
    File file = LittleFS.open("/telemetry.jsonl", FILE_APPEND);
    if (file) {
      file.println(line);
      file.flush();
      file.close();
      written = true;
    }
  }

  return written;
}

bool Storage::readConfig(char* out, size_t n) {
  if (!out || n == 0) return false;

  File file;
  if (_sdPresent && SD.exists("/config.json")) {
    file = SD.open("/config.json", FILE_READ);
  } else if (_fsPresent && LittleFS.exists("/config.json")) {
    file = LittleFS.open("/config.json", FILE_READ);
  }

  if (file) {
    size_t readBytes = file.readBytes(out, n - 1);
    out[readBytes] = '\0';
    file.close();
    return true;
  }

  // Default JSON vazio se arquivo não existir
  snprintf(out, n, "{}");
  return false;
}

bool Storage::writeConfig(const char* json) {
  if (!json) return false;

  bool written = false;

  if (_sdPresent) {
    File file = SD.open("/config.json", FILE_WRITE);
    if (file) {
      file.print(json);
      file.flush();
      file.close();
      written = true;
    }
  }

  if (_fsPresent) {
    File file = LittleFS.open("/config.json", FILE_WRITE);
    if (file) {
      file.print(json);
      file.flush();
      file.close();
      written = true;
    }
  }

  return written;
}

bool Storage::healthy() const {
  if (_sdPresent) {
    return (SD.cardType() != CARD_NONE);
  }
  return _fsPresent;
}

}
