#pragma once
#include <Arduino.h>
#include <math.h>

// ═══════════════════════════════════════════════════════════════════════════
// NilmDatabase — biblioteca de equipamentos + classificador (CHG-20260906-004)
// PORTE FIEL do V11.9 (DB_EQUIP L203-224, PERIODO_NOME L225-227, getPeriodo
// L341-346, classificarCarga L448-467). Conhecimento REAL do sistema — não
// hipótese — base da KB-001 e do classificador do LearningEngine.
// Ajustes por instalação ficam no Asset DNA; esta é a base inicial residencial.
// ═══════════════════════════════════════════════════════════════════════════

namespace proteve {
namespace nilm {

enum Periodo : uint8_t { P_MADRUGADA = 0, P_CAFE, P_MANHA, P_ALMOCO, P_TARDE, P_JANTAR, P_SONO, P_COUNT };
static const char* PERIODO_NOME[P_COUNT] = {
  "Madrugada", "Cafe da manha", "Manha", "Almoco", "Tarde", "Jantar", "Sono/Noite"
};

// tipo: 0 = resistivo/eletrônico FP alto · 1 = motor/compressor · 2 = eletrônico FP baixo
struct Equipamento {
  const char* nome;
  float pMin, pMax, pfMin;
  uint8_t tipo;
  uint16_t duracaoMedia;      // segundos (aprendida — atualizarDuracao do V11.9)
  float pesoPeriodo[7];
  bool fimDeSemana;
};

// 14 equipamentos — valores EXATOS do V11.9 (DB_EQUIP)
static const Equipamento DB_EQUIP[] = {
  {"Ar-condicionado",  600, 2800, 0.58, 1,  7200, {1.5f,0.8f,1.2f,0.8f,1.8f,1.2f,2.0f}, false},
  {"Chuveiro",        3000, 7500, 0.92, 0,   480, {0.1f,2.0f,1.5f,0.3f,0.4f,2.0f,0.3f}, false},
  {"Sanduicheira",     700, 1500, 0.90, 0,   600, {0.0f,2.0f,0.8f,0.2f,0.1f,0.4f,0.1f}, false},
  {"Chaleira",         900, 1600, 0.92, 0,   180, {0.1f,2.0f,1.0f,1.2f,0.5f,1.0f,0.3f}, false},
  {"Airfryer",        1200, 2000, 0.88, 0,   900, {0.0f,1.8f,0.4f,1.8f,0.3f,1.8f,0.2f}, false},
  {"Microondas",       900, 1800, 0.52, 2,   120, {0.0f,0.5f,0.3f,2.0f,0.4f,2.0f,0.3f}, false},
  {"Forno eletrico",  1800, 3500, 0.92, 0,  1800, {0.0f,0.3f,0.3f,1.8f,0.5f,1.8f,0.2f}, false},
  {"Exaustor",          80,  400, 0.58, 1,  1200, {0.0f,0.3f,0.3f,1.8f,0.3f,1.8f,0.2f}, false},
  {"Ferro de passar", 1000, 2500, 0.92, 0,  1800, {0.0f,0.5f,1.5f,0.3f,1.5f,0.4f,0.2f}, false},
  {"Bomba de piscina",  500, 1500, 0.62, 1,  3600, {0.0f,0.3f,1.5f,0.8f,1.5f,0.3f,0.0f}, true },
  {"Portao eletronico",150,  500, 0.55, 1,    15, {0.1f,1.5f,1.5f,1.2f,1.5f,1.5f,0.3f}, false},
  {"Geladeira (ciclo)",  80,  380, 0.50, 1,   900, {1.2f,1.0f,1.0f,1.0f,1.0f,1.0f,1.2f}, false},
  {"TV / informatica",  30,  400, 0.55, 2,  7200, {0.5f,0.5f,0.8f,1.5f,1.5f,1.8f,1.8f}, false},
  {"Iluminacao",          5,  250, 0.80, 0, 14400, {0.8f,1.0f,0.8f,0.8f,0.8f,1.2f,1.5f}, false},
};
static const int N_EQUIP = (int)(sizeof(DB_EQUIP) / sizeof(DB_EQUIP[0]));

// getPeriodo (V11.9 L341-346)
inline Periodo getPeriodo(int h) {
  if (h < 5)  return P_MADRUGADA;
  if (h < 9)  return P_CAFE;
  if (h < 11) return P_MANHA;
  if (h < 14) return P_ALMOCO;
  if (h < 18) return P_TARDE;
  if (h < 21) return P_JANTAR;
  return P_SONO;
}

// classificarCarga (V11.9 L448-467) — score = centralidade × multPF × pesoPer × multFDS
// Retorna índice em DB_EQUIP ou -1 (limiar NILM_THRESHOLD=40W já aplicado fora).
struct NilmMatch { int idx; float score; };

inline NilmMatch classificarCarga(float dP, float pfAtual, int periodo, bool fds) {
  NilmMatch best{-1, -1.0f};
  if (fabsf(dP) < 40.0f) return best;             // NILM_THRESHOLD do V11.9
  float ap = fabsf(dP);
  for (int i = 0; i < N_EQUIP; i++) {
    const Equipamento& e = DB_EQUIP[i];
    if (ap < e.pMin || ap > e.pMax) continue;
    float centro = (e.pMin + e.pMax) / 2.0f, faixa = (e.pMax - e.pMin) / 2.0f;
    float centralidade = 1.0f - fabsf(ap - centro) / faixa;
    float multPF = 1.0f;
    if (pfAtual > 0.05f) {
      if (e.tipo == 0) { multPF = (pfAtual >= 0.90f) ? 1.4f : (pfAtual >= 0.80f) ? 1.0f : (pfAtual >= 0.65f) ? 0.5f : 0.1f; }
      else if (e.tipo == 1) { multPF = (pfAtual >= 0.55f && pfAtual <= 0.80f) ? 1.4f : (pfAtual > 0.80f && pfAtual < 0.92f) ? 0.7f : (pfAtual >= 0.92f) ? 0.2f : 0.8f; }
      else { multPF = (pfAtual >= 0.50f && pfAtual <= 0.82f) ? 1.3f : (pfAtual > 0.82f) ? 0.6f : 0.7f; }
    }
    float pesoPer = e.pesoPeriodo[periodo];
    float multFDS = e.fimDeSemana ? (fds ? 1.5f : 0.3f) : 1.0f;
    float score = centralidade * multPF * pesoPer * multFDS;
    if (score > best.score) { best.score = score; best.idx = i; }
  }
  if (best.score < 0.05f) { best.idx = -1; best.score = 0; }   // limiar do V11.9
  return best;
}

// Nota R-02: no MVP a saída do NILM é sempre "provável + confiança + evidências"
// (LearningEngine/LearningEngine.h inferLoads). O classificador retorna score —
// confiança normalizada é calculada por quem consome, nunca verdade absoluta.
// A correção humana (CONFIRMAR/CORRIGIR) alimenta o Asset DNA (RECONSTRUCTION §9).

} // namespace nilm
} // namespace proteve
