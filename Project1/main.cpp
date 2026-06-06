#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <ctime>

// Biblioteki z Twoich zajęć uczelnianych
#include "lodepng.h"
#include "shaderprogram.h"

// Nasze klasy obiektów
#include "Cube.h" 
#include "Fish.h" 
#include "Rock.h"
#include "Plant.h"
#define PI 3.14159265358979323846f

float speed_x = 0; float speed_y = 0; float speed_zoom = 0; float fov = 50.0f;

Cube* aquariumBox = nullptr;
ShaderProgram* sp = nullptr;

GLuint texSand; GLuint texRock;

// ==========================================
// --- GŁÓWNE PARAMETRY SCENY (Do edycji!) ---

const int NUM_FISHES = 15;   // Zmniejszone — mniej tłoczno
const int NUM_ROCKS = 10;
const int NUM_PLANTS = 12;   // Więcej roślin
float lightIntensity = 1.5f; // Jaśniej


std::vector<Fish*> aquariumFishes;
std::vector<Rock*> aquariumRocks;
std::vector<Plant*> aquariumPlants;

GLuint readTexture(const char* filename) {
    GLuint tex; glActiveTexture(GL_TEXTURE0);
    std::vector<unsigned char> image; unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filename);
    if (error) printf("Decoder error %d: %s\n", error, lodepng_error_text(error));
    glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

float getRandomFloat(float min, float max) { return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min)); }
void error_callback(int error, const char* description) { fputs(description, stderr); }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT)  speed_y = -PI;
        if (key == GLFW_KEY_RIGHT) speed_y = PI;
        if (key == GLFW_KEY_UP)    speed_x = -PI;
        if (key == GLFW_KEY_DOWN)  speed_x = PI;
        if (key == GLFW_KEY_O) speed_zoom = 30.0f;
        if (key == GLFW_KEY_P) speed_zoom = -30.0f;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) speed_y = 0;
        if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN)    speed_x = 0;
        if (key == GLFW_KEY_O || key == GLFW_KEY_P)        speed_zoom = 0;
    }
}

void initOpenGLProgram(GLFWwindow* window) {
    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

    glClearColor(0.05f, 0.09f, 0.14f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    glfwSetKeyCallback(window, key_callback);
    aquariumBox = new Cube();
    texSand = readTexture("sand.png");
    texRock = readTexture("rock.png");

    srand(static_cast<unsigned int>(time(0)));
    glm::vec3 corner(-2.4f, -1.4f, -1.4f);

    // Generacja rybek (używamy zmiennej NUM_FISHES)
    for (int i = 0; i < NUM_FISHES; i++) {
        glm::vec3 randomOffset(getRandomFloat(0.0f, 4.8f), getRandomFloat(0.0f, 2.5f), getRandomFloat(0.0f, 2.8f));
        glm::vec3 pos = corner + randomOffset;
        glm::vec3 vel(getRandomFloat(-2.0f, 2.0f), getRandomFloat(-0.5f, 0.5f), getRandomFloat(-2.0f, 2.0f));
        float r = getRandomFloat(0.0f, 1.0f); float g = getRandomFloat(0.0f, 1.0f); float b = getRandomFloat(0.0f, 1.0f);
        if (r > 0.6f && b > 0.6f && g < 0.5f) { g = 0.8f; }
        aquariumFishes.push_back(new Fish(pos, glm::vec3(r, g, b), vel));
    }

    // Generacja kamieni (używamy zmiennej NUM_ROCKS)
    for (int i = 0; i < NUM_ROCKS; i++) {
        glm::vec3 pos(getRandomFloat(-2.2f, 2.2f), -1.8f, getRandomFloat(-1.2f, 1.2f));
        glm::vec3 rockScale(getRandomFloat(0.3f, 0.6f), getRandomFloat(0.1f, 0.2f), getRandomFloat(0.3f, 0.6f));
        aquariumRocks.push_back(new Rock(pos, rockScale));
    }
    // Generacja roślin na dnie akwarium
    // Generacja roślin — przy ściankach i w skupiskach
    for (int i = 0; i < NUM_PLANTS; i++) {
        // Rośliny przy ściankach bocznych i z przodu/tyłu
        float px, pz;
        if (i % 3 == 0) {
            // Przy lewej/prawej ściance
            px = (i % 2 == 0) ? getRandomFloat(-2.5f, -1.5f)
                : getRandomFloat(1.5f, 2.5f);
            pz = getRandomFloat(-1.4f, 1.4f);
        }
        else {
            // Losowo na dnie
            px = getRandomFloat(-2.3f, 2.3f);
            pz = getRandomFloat(-1.5f, 1.5f);
        }
        glm::vec3 pos(px, -1.95f, pz);
        float height = getRandomFloat(0.5f, 1.1f); // Wyższe
        float swayStrength = getRandomFloat(0.20f, 0.40f);
        aquariumPlants.push_back(new Plant(pos, height, swayStrength));
    }
}

void freeOpenGLProgram(GLFWwindow* window) {
    if (aquariumBox != nullptr) delete aquariumBox;
    for (Fish* fish : aquariumFishes) delete fish;
    for (Rock* rock : aquariumRocks) delete rock;
    for (Plant* plant : aquariumPlants) delete plant;
    glDeleteTextures(1, &texSand); glDeleteTextures(1, &texRock);
    if (sp != nullptr) delete sp;
}

void drawScene(GLFWwindow* window, float angle_x, float angle_y,
    float current_fov) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Kamera — oryginalna pozycja
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -15.0f);
    glm::mat4 V = glm::lookAt(cameraPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 P = glm::perspective(glm::radians(current_fov),
        1024.0f / 768.0f, 0.1f, 100.0f);

    glm::mat4 baseM = glm::mat4(1.0f);
    baseM = glm::rotate(baseM, angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    baseM = glm::rotate(baseM, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

    sp->use();

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    GLuint shaderProgramID = (GLuint)currentProgram;

    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform1f(sp->u("lightIntensity"), lightIntensity);

    // ══════════════════════════════════════════
    //  ETAP 1: OBIEKTY NIEPRZEZROCZYSTE
    // ══════════════════════════════════════════
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Reset flag shadera
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("useProceduralFish"), 0);
    glUniform1i(sp->u("useProceduralPlant"), 0);
    glUniform1i(sp->u("isRock"), 0);
    glUniform1i(sp->u("isEmissive"), 0);

    // ─── MODEL LAMPY AKWARIOWEJ ────────────────────────────────────
// Płaski pasek LED nad akwarium — rysowany jako emissive (świeci)
    glUniform1i(sp->u("isEmissive"), 1);
    glm::mat4 lampM = baseM;
    lampM = glm::translate(lampM, glm::vec3(0.0f, 2.10f, 0.0f));
    lampM = glm::scale(lampM, glm::vec3(0.88f, 0.022f, 0.10f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(lampM));
    // Biało-żółty blask — ciepłe LED
    glUniform4f(sp->u("objectColor"), 1.0f, 0.97f, 0.75f, 1.0f);
    aquariumBox->drawSolid();
    glUniform1i(sp->u("isEmissive"), 0);   // reset po lampie
    // ──────────────────────────────────────────────────────────────


    // --- Dno akwarium z piaskiem ---
    // Skala oryginalna (1.1 w X/Z = trochę szersza niż akwarium)
    glm::mat4 standM = baseM;
    standM = glm::translate(standM, glm::vec3(0.0f, -2.1f, 0.0f));
    standM = glm::scale(standM, glm::vec3(1.05f, 0.10f, 1.05f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(standM));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSand);
    glUniform1i(sp->u("tex"), 0);
    glUniform1i(sp->u("useTexture"), 1);
    glUniform1i(sp->u("isRock"), 0);
    glUniform4f(sp->u("objectColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    aquariumBox->drawSolid();

    // --- Drewniana podstawa pod akwarium ---
    // Skala 1.12 w X/Z: Cube ma X od -3 do 3, więc 1.12*6=6.72 vs akwarium 6.0
    // Efekt: listwa drewniana wychodząca 0.36 z każdej strony
    glm::mat4 woodM = baseM;
    woodM = glm::translate(woodM, glm::vec3(0.0f, -2.28f, 0.0f));
    woodM = glm::scale(woodM, glm::vec3(1.12f, 0.08f, 1.12f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(woodM));
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("isRock"), 0);
    // Ciepły brązowy kolor drewna
    glUniform4f(sp->u("objectColor"), 0.45f, 0.28f, 0.08f, 1.0f);
    aquariumBox->drawSolid();

    // --- Kamienie (proceduralny wzór skały) ---
    glBindTexture(GL_TEXTURE_2D, texRock);
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("isRock"), 1);  // <-- włącz wzór kamienia
    glUniform4f(sp->u("objectColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    for (Rock* rock : aquariumRocks) {
        rock->draw(shaderProgramID, baseM);
    }
    glUniform1i(sp->u("isRock"), 0);     // <-- reset po kamieniach

    // --- Rybki (łuski proceduralne w Fish::draw) ---
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("useProceduralFish"), 0);
    glUniform1i(sp->u("useProceduralPlant"), 0);
    for (Fish* fish : aquariumFishes) {
        fish->draw(shaderProgramID, baseM);
    }

    // ══════════════════════════════════════════
    //  ETAP 2: ROŚLINY (lekko przezroczyste)
    // ══════════════════════════════════════════
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("useProceduralFish"), 0);
    glUniform1i(sp->u("isRock"), 0);
    for (Plant* plant : aquariumPlants) {
        plant->draw(shaderProgramID, baseM);
    }

    // ══════════════════════════════════════════
    //  ETAP 3: SZKIELET AKWARIUM
    // ══════════════════════════════════════════
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("useProceduralFish"), 0);
    glUniform1i(sp->u("useProceduralPlant"), 0);
    glUniform1i(sp->u("isRock"), 0);
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(baseM));
    glUniform4f(sp->u("objectColor"), 0.08f, 0.08f, 0.08f, 1.0f);
    aquariumBox->drawEdges();

    // ══════════════════════════════════════════
    //  ETAP 4: PRZEZROCZYSTE (woda + szkło)
    // ══════════════════════════════════════════
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("useProceduralFish"), 0);
    glUniform1i(sp->u("useProceduralPlant"), 0);
    glUniform1i(sp->u("isRock"), 0);

    // Woda (niebieskawa, przezroczysta)
    glm::mat4 waterM = baseM;
    waterM = glm::translate(waterM, glm::vec3(0.0f, -0.1f, 0.0f));
    waterM = glm::scale(waterM, glm::vec3(0.99f, 0.93f, 0.99f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(waterM));
    glUniform4f(sp->u("objectColor"), 0.04f, 0.22f, 0.58f, 0.22f);
    aquariumBox->drawSolid();

    // Szkło (bardzo delikatne)
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(baseM));
    glUniform4f(sp->u("objectColor"), 0.80f, 0.90f, 1.00f, 0.07f);
    aquariumBox->drawSolid();

    glDepthMask(GL_TRUE);
    glfwSwapBuffers(window);
}
int main(void) {
    GLFWwindow* window; glfwSetErrorCallback(error_callback);
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 768, "Aquarium Project", NULL, NULL);
    if (!window) { glfwTerminate(); exit(EXIT_FAILURE); }
    glfwMakeContextCurrent(window); glfwSwapInterval(1);
    glewExperimental = GL_TRUE; if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);
    initOpenGLProgram(window);
    float angle_x = 0; float angle_y = 0; glfwSetTime(0);

    while (!glfwWindowShouldClose(window)) {
        float deltaTime = glfwGetTime(); glfwSetTime(0);
        angle_x += speed_x * deltaTime; angle_y += speed_y * deltaTime;
        fov += speed_zoom * deltaTime;
        if (fov < 10.0f) fov = 10.0f; if (fov > 120.0f) fov = 120.0f;
        for (Fish* fish : aquariumFishes) { fish->update(deltaTime); 
        for (Plant* plant : aquariumPlants) { plant->update(deltaTime); }
        }
        drawScene(window, angle_x, angle_y, fov);
        glfwPollEvents();
    }
    freeOpenGLProgram(window); glfwDestroyWindow(window); glfwTerminate(); exit(EXIT_SUCCESS);
}