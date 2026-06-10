#include "params.h"

// ===== ILE CZEGO W AKWARIUM =====
int NUM_FISHES  = 15;   // liczba rybek
int NUM_ROCKS   = 10;   // liczba kamieni
int NUM_PLANTS  = 10;   // liczba roslin (kazda to kepa kilku lodyg)
int NUM_BUBBLES = 28;   // liczba babelkow

// ===== JASNOSC =====
float lightIntensity = 0.60f;   // ogolna jasnosc sceny

// ===== DWA DOLNE KOLOROWE SWIATLA (kuleczki) =====
glm::vec3 kolor1  = glm::vec3(0.10f, 0.40f, 1.00f);   // domyslnie niebieskie
glm::vec3 kolor2  = glm::vec3(1.00f, 0.12f, 0.06f);   // domyslnie czerwone
glm::vec3 botPos1 = glm::vec3(-1.3f, -1.62f, -0.2f);  // pozycja kuleczki 1
glm::vec3 botPos2 = glm::vec3( 1.3f, -1.62f, -0.2f);  // pozycja kuleczki 2

// ===== PULSOWANIE DIOD =====
float pulseStrength = 0.30f;    // sila pulsowania
float pulseSpeed    = 3.0f;     // tempo pulsowania

// ===== WCZYTANY MODEL (pistolet) =====
glm::vec3 MODEL_POS   = glm::vec3(0.0f, -1.80f, -1.1f);  // pozycja na dnie (zmien Y jesli wpada/lewituje)
float     MODEL_SCALE = 0.9f;                            // rozmiar
float     MODEL_ROT_X = 90.0f;                           // 90 = lezy poziomo

// ===== WIELKOSC AKWARIUM =====
float aquariumScale = 4.0f;     // 1.0 = bazowa; wiecej = wieksze akwarium (cala scena skaluje sie razem)
