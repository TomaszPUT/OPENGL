#pragma once
#include <glm/glm.hpp>

// ============================================================
//  Proste parametry sceny - wszystko, co latwo zmienic i wytlumaczyc.
//  Wartosci ustawia sie w params.cpp.
// ============================================================

// Ile czego w akwarium
extern int NUM_FISHES;
extern int NUM_ROCKS;
extern int NUM_PLANTS;
extern int NUM_BUBBLES;

// Ogolna jasnosc sceny (w trakcie: Q = jasniej, E = ciemniej)
extern float lightIntensity;

// Dwa dolne kolorowe swiatla (swiecace kuleczki): kolory i pozycje
extern glm::vec3 kolor1;
extern glm::vec3 kolor2;
extern glm::vec3 botPos1;
extern glm::vec3 botPos2;

// Pulsowanie diod
extern float pulseStrength;   // 0 = stale swiatlo, 1 = pelne miganie
extern float pulseSpeed;      // wieksze = szybsze

// Wczytany model (pistolet) lezacy na dnie
extern glm::vec3 MODEL_POS;
extern float     MODEL_SCALE;
extern float     MODEL_ROT_X;

// Wielkosc calego akwarium (skaluje cala scene: 1.0 = bazowa, 1.5 = wieksze itd.)
extern float aquariumScale;

// Efekt "mydlanej banki": 0 = babelki jak dawniej (rownomierne), 1 = srodek prawie
// calkiem przezroczysty, widac glownie swiecacy rant.
extern float bubbleClear;
