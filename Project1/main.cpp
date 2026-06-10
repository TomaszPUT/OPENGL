#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <ctime>
#include <cmath>

#include "lodepng.h"
#include "shaderprogram.h"
#include "params.h"

#include "Cube.h"
#include "Fish.h"
#include "Rock.h"
#include "Plant.h"
#include "Model.h"
#include "Bubbles.h"
#define PI 3.14159265358979323846f

// Sterowanie kamera i klawiszami
float speed_x = 0; float speed_y = 0; float speed_zoom = 0; float fov = 50.0f;
bool  keyLightUp = false, keyLightDown = false, keyHue = false;   // Q / E / i
float hueOffset = 0.0f;   // obrot odcienia diod (klawisz i)

float camPitch = 0.0f, camYaw = 0.0f;          // katy kamery (strzalki lub mysz)
bool  mouseDragging = false; double lastMouseX = 0.0, lastMouseY = 0.0;

Cube* aquariumBox = nullptr;
ShaderProgram* sp = nullptr;
Model* shellModel = nullptr;
Bubbles* bubbles = nullptr;
float appTime = 0.0f;     // czas animacji

GLuint texSand; GLuint texRock;

// Wspoldzielona kula uzywana do dwoch swiecacych kuleczek (dolne swiatla)
GLuint ballVAO = 0, ballVBO = 0, ballEBO = 0; int ballCount = 0;

// Skupiska, wokol ktorych rozmieszczamy kamienie i rosliny
const glm::vec2 CLUSTERS[3] = { glm::vec2(-1.5f, 0.4f), glm::vec2(1.4f, -0.4f), glm::vec2(0.3f, 1.0f) };

std::vector<Fish*> aquariumFishes;
std::vector<Rock*> aquariumRocks;
std::vector<Plant*> aquariumPlants;

void makeLightBall() {
    std::vector<float> v; std::vector<unsigned int> idx;
    int sectors = 16, stacks = 12; float r = 1.0f;
    for (int i = 0; i <= stacks; ++i) {
        float sa = 3.14159265f / 2 - i * 3.14159265f / stacks; float y = r * sinf(sa);
        for (int j = 0; j <= sectors; ++j) {
            float se = j * 2 * 3.14159265f / sectors;
            float x = r * cosf(sa) * cosf(se), z = r * cosf(sa) * sinf(se);
            v.push_back(x); v.push_back(y); v.push_back(z);
            v.push_back(0); v.push_back(0);
            v.push_back(x); v.push_back(y); v.push_back(z);
        }
    }
    for (int i = 0; i < stacks; ++i) for (int j = 0; j < sectors; ++j) {
        unsigned int k1 = i * (sectors + 1) + j, k2 = k1 + sectors + 1;
        idx.push_back(k1); idx.push_back(k2); idx.push_back(k1 + 1);
        idx.push_back(k1 + 1); idx.push_back(k2); idx.push_back(k2 + 1);
    }
    ballCount = (int)idx.size();
    glGenVertexArrays(1, &ballVAO); glGenBuffers(1, &ballVBO); glGenBuffers(1, &ballEBO);
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

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
        if (key == GLFW_KEY_Q) keyLightUp = true;
        if (key == GLFW_KEY_E) keyLightDown = true;
        if (key == GLFW_KEY_I) keyHue = true;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) speed_y = 0;
        if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN)    speed_x = 0;
        if (key == GLFW_KEY_O || key == GLFW_KEY_P)        speed_zoom = 0;
        if (key == GLFW_KEY_Q) keyLightUp = false;
        if (key == GLFW_KEY_E) keyLightDown = false;
        if (key == GLFW_KEY_I) keyHue = false;
    }
}

// Prawy przycisk myszy: przytrzymaj i ruszaj, aby obracac kamere
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            mouseDragging = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            mouseDragging = false;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double x, double y) {
    if (!mouseDragging) return;
    double dx = x - lastMouseX, dy = y - lastMouseY;
    lastMouseX = x; lastMouseY = y;
    camYaw   += (float)dx * 0.005f;     // czulosc obrotu
    camPitch += (float)dy * 0.005f;
    if (camPitch >  1.40f) camPitch =  1.40f;
    if (camPitch < -1.40f) camPitch = -1.40f;
}

// Obrot odcienia koloru o zadany kat (RGB -> HSV -> przesun H -> RGB).
glm::vec3 rotateHue(glm::vec3 c, float deg) {
    float mx = glm::max(c.r, glm::max(c.g, c.b));
    float mn = glm::min(c.r, glm::min(c.g, c.b));
    float d = mx - mn;
    float h = 0.0f, s = (mx <= 0.0f) ? 0.0f : d / mx, v = mx;
    if (d > 0.0f) {
        if (mx == c.r)      h = 60.0f * fmodf(((c.g - c.b) / d), 6.0f);
        else if (mx == c.g) h = 60.0f * (((c.b - c.r) / d) + 2.0f);
        else                h = 60.0f * (((c.r - c.g) / d) + 4.0f);
    }
    h = fmodf(h + deg, 360.0f); if (h < 0.0f) h += 360.0f;
    float cc = v * s, x = cc * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f)), m = v - cc;
    glm::vec3 r;
    if (h < 60)       r = glm::vec3(cc, x, 0);
    else if (h < 120) r = glm::vec3(x, cc, 0);
    else if (h < 180) r = glm::vec3(0, cc, x);
    else if (h < 240) r = glm::vec3(0, x, cc);
    else if (h < 300) r = glm::vec3(x, 0, cc);
    else              r = glm::vec3(cc, 0, x);
    return r + glm::vec3(m);
}

void initOpenGLProgram(GLFWwindow* window) {
    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

    glClearColor(0.03f, 0.07f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    aquariumBox = new Cube();
    texSand = readTexture("sand.png");
    texRock = readTexture("rock.png");

    srand(static_cast<unsigned int>(time(0)));

    shellModel = new Model("shell.obj");
    bubbles    = new Bubbles(NUM_BUBBLES);
    makeLightBall();

    glm::vec3 corner(-2.4f, -1.4f, -1.4f);

    // Rybki - plywaja swobodnie po calym akwarium
    for (int i = 0; i < NUM_FISHES; i++) {
        glm::vec3 randomOffset(getRandomFloat(0.0f, 4.8f), getRandomFloat(0.0f, 2.5f), getRandomFloat(0.0f, 2.8f));
        glm::vec3 pos = corner + randomOffset;
        glm::vec3 vel(getRandomFloat(-2.0f, 2.0f), getRandomFloat(-0.5f, 0.5f), getRandomFloat(-2.0f, 2.0f));
  
        float r = 0.0f; float g = getRandomFloat(0.0f, 1.0f); float b = getRandomFloat(0.0f, 1.0f);
        if (r > 0.6f && b > 0.6f && g < 0.5f) { g = 0.8f; }   // bez koloru rozowego
        aquariumFishes.push_back(new Fish(pos, glm::vec3(r, g, b), vel));
    }

    // Kamienie - w skupiskach; pierwszy wiekszy jako centralny glaz
    for (int i = 0; i < NUM_ROCKS; i++) {
        glm::vec2 c = CLUSTERS[i % 3];
        glm::vec3 pos(c.x + getRandomFloat(-0.6f, 0.6f), -1.8f, c.y + getRandomFloat(-0.5f, 0.5f));
        glm::vec3 rockScale;
        if (i == 0) rockScale = glm::vec3(0.75f, 0.5f, 0.75f);
        else        rockScale = glm::vec3(getRandomFloat(0.3f, 0.55f), getRandomFloat(0.15f, 0.30f), getRandomFloat(0.3f, 0.55f));
        aquariumRocks.push_back(new Rock(pos, rockScale));
    }

    // Rosliny - w tych samych skupiskach
    for (int i = 0; i < NUM_PLANTS; i++) {
        glm::vec2 c = CLUSTERS[i % 3];
        glm::vec3 pos(c.x + getRandomFloat(-1.0f, 1.0f), -1.95f, c.y + getRandomFloat(-0.9f, 0.9f));
        float height = getRandomFloat(0.6f, 1.2f);
        float swayStrength = getRandomFloat(0.20f, 0.40f);
        aquariumPlants.push_back(new Plant(pos, height, swayStrength));
    }
}

void freeOpenGLProgram(GLFWwindow* window) {
    if (aquariumBox != nullptr) delete aquariumBox;
    if (shellModel != nullptr) delete shellModel;
    if (bubbles != nullptr) delete bubbles;
    for (Fish* fish : aquariumFishes) delete fish;
    for (Rock* rock : aquariumRocks) delete rock;
    for (Plant* plant : aquariumPlants) delete plant;
    glDeleteTextures(1, &texSand); glDeleteTextures(1, &texRock);
    glDeleteVertexArrays(1, &ballVAO); glDeleteBuffers(1, &ballVBO); glDeleteBuffers(1, &ballEBO);
    if (sp != nullptr) delete sp;
}

// Ustawia wszystkie flagi materialu na 0
void resetFlags() {
    glUniform1i(sp->u("useTexture"), 0);
    glUniform1i(sp->u("useProceduralFish"), 0);
    glUniform1i(sp->u("useProceduralPlant"), 0);
    glUniform1i(sp->u("isRock"), 0);
    glUniform1i(sp->u("isEmissive"), 0);
    glUniform1i(sp->u("isPearl"), 0);
    glUniform1i(sp->u("isBubble"), 0);
    glUniform1i(sp->u("isGold"), 0);
}

void drawScene(GLFWwindow* window, float angle_x, float angle_y, float current_fov) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Kamera orbitujaca: scena stoi, obraca sie widok wokol srodka.
    // Wszystko skalujemy o aquariumScale (wieksze akwarium -> kamera odsuwa sie tak samo).
    glm::vec3 center = glm::vec3(0.0f, -0.2f, 0.0f) * aquariumScale;
    float pitch = angle_x;
    float yaw   = angle_y;
    float radius = 13.0f * aquariumScale;
    glm::vec3 cameraPos = center + radius * glm::vec3(
        cosf(pitch) * sinf(yaw),
        sinf(pitch),
        cosf(pitch) * cosf(yaw));
    glm::mat4 V = glm::lookAt(cameraPos, center, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 P = glm::perspective(glm::radians(current_fov), 1024.0f / 768.0f, 0.1f, 100.0f * aquariumScale);

    glm::mat4 baseM = glm::scale(glm::mat4(1.0f), glm::vec3(aquariumScale));   // skala calej sceny

    sp->use();
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    GLuint shaderProgramID = (GLuint)currentProgram;

    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform1f(sp->u("lightIntensity"), lightIntensity);
    glUniform1f(sp->u("time"), appTime);
    glUniform3fv(sp->u("camPos"), 1, glm::value_ptr(cameraPos));
    glUniform1f(sp->u("pulseStrength"), pulseStrength);
    glUniform1f(sp->u("pulseSpeed"), pulseSpeed);
    glUniform1f(sp->u("sceneScale"), aquariumScale);
    glUniform1f(sp->u("bubbleClear"), bubbleClear);

    // Kolory diod po obrocie odcienia (klawisz i); pozycje skalujemy razem ze scena
    glm::vec3 effCol1 = rotateHue(kolor1, hueOffset);
    glm::vec3 effCol2 = rotateHue(kolor2, hueOffset);
    glUniform3fv(sp->u("botLight1Pos"), 1, glm::value_ptr(botPos1 * aquariumScale));
    glUniform3fv(sp->u("botLight1Col"), 1, glm::value_ptr(effCol1));
    glUniform3fv(sp->u("botLight2Pos"), 1, glm::value_ptr(botPos2 * aquariumScale));
    glUniform3fv(sp->u("botLight2Col"), 1, glm::value_ptr(effCol2));

    // ===== ETAP 1: OBIEKTY NIEPRZEZROCZYSTE =====
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    resetFlags();

    // Gorna lampa (swieci)
    glUniform1i(sp->u("isEmissive"), 1);
    glm::mat4 lampM = baseM;
    lampM = glm::translate(lampM, glm::vec3(0.0f, 2.12f, 0.0f));
    lampM = glm::scale(lampM, glm::vec3(0.95f, 0.030f, 0.30f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(lampM));
    glUniform4f(sp->u("objectColor"), 1.0f, 0.97f, 0.85f, 1.0f);
    aquariumBox->drawSolid();
    glUniform1i(sp->u("isEmissive"), 0);

    // Dwie swiecace kuleczki na dnie (z pulsowaniem)
    float ballPulse = (1.0f - pulseStrength) + pulseStrength * (0.5f + 0.5f * sinf(appTime * pulseSpeed));
    glUniform1i(sp->u("isEmissive"), 1);
    glBindVertexArray(ballVAO);
    glm::mat4 ball1 = glm::scale(glm::translate(baseM, botPos1), glm::vec3(0.13f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(ball1));
    glUniform4f(sp->u("objectColor"), effCol1.r * ballPulse, effCol1.g * ballPulse, effCol1.b * ballPulse, 1.0f);
    glDrawElements(GL_TRIANGLES, ballCount, GL_UNSIGNED_INT, 0);
    glm::mat4 ball2 = glm::scale(glm::translate(baseM, botPos2), glm::vec3(0.13f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(ball2));
    glUniform4f(sp->u("objectColor"), effCol2.r * ballPulse, effCol2.g * ballPulse, effCol2.b * ballPulse, 1.0f);
    glDrawElements(GL_TRIANGLES, ballCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUniform1i(sp->u("isEmissive"), 0);

    // Dno z piaskiem
    glm::mat4 standM = baseM;
    standM = glm::translate(standM, glm::vec3(0.0f, -2.1f, 0.0f));
    standM = glm::scale(standM, glm::vec3(1.05f, 0.10f, 1.05f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(standM));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texSand);
    glUniform1i(sp->u("tex"), 0);
    glUniform1i(sp->u("useTexture"), 1);
    glUniform4f(sp->u("objectColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    aquariumBox->drawSolid();
    glUniform1i(sp->u("useTexture"), 0);

    // Zlota podstawa pod akwarium
    glm::mat4 woodM = baseM;
    woodM = glm::translate(woodM, glm::vec3(0.0f, -2.28f, 0.0f));
    woodM = glm::scale(woodM, glm::vec3(1.12f, 0.08f, 1.12f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(woodM));
    glUniform1i(sp->u("isGold"), 1);
    glUniform4f(sp->u("objectColor"), 0.62f, 0.48f, 0.16f, 1.0f);
    aquariumBox->drawSolid();
    glUniform1i(sp->u("isGold"), 0);

    // Kamienie
    glBindTexture(GL_TEXTURE_2D, texRock);
    glUniform1i(sp->u("tex"), 0);
    glUniform1i(sp->u("isRock"), 1);
    glUniform4f(sp->u("objectColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    for (Rock* rock : aquariumRocks) rock->draw(shaderProgramID, baseM);
    glUniform1i(sp->u("isRock"), 0);

    // Wczytany model (pistolet) - lezy na dnie, rysowany jako szary metal
    {
        glm::mat4 Mm = baseM;
        Mm = glm::translate(Mm, MODEL_POS);
        Mm = glm::rotate(Mm, glm::radians(MODEL_ROT_X), glm::vec3(1.0f, 0.0f, 0.0f));
        Mm = glm::scale(Mm, glm::vec3(MODEL_SCALE));
        glUniform4f(sp->u("objectColor"), 0.55f, 0.56f, 0.60f, 1.0f);
        shellModel->draw(shaderProgramID, Mm);
    }

    // Rybki
    for (Fish* fish : aquariumFishes) fish->draw(shaderProgramID, baseM);

    // ===== ETAP 2: ROSLINY =====
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    resetFlags();
    for (Plant* plant : aquariumPlants) plant->draw(shaderProgramID, baseM);

    // ===== ETAP 3: SZKIELET AKWARIUM =====
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    resetFlags();
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(baseM));
    glUniform4f(sp->u("objectColor"), 0.08f, 0.08f, 0.08f, 1.0f);
    aquariumBox->drawEdges();

    // ===== ETAP 4: PRZEZROCZYSTE (babelki + woda + szklo) =====
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    resetFlags();

    bubbles->draw(shaderProgramID, baseM);

    // Woda
    glm::mat4 waterM = baseM;
    waterM = glm::translate(waterM, glm::vec3(0.0f, -0.1f, 0.0f));
    waterM = glm::scale(waterM, glm::vec3(0.99f, 0.93f, 0.99f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(waterM));
    glUniform4f(sp->u("objectColor"), 0.04f, 0.20f, 0.55f, 0.24f);
    aquariumBox->drawSolid();

    // Szklo
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
    glfwSetTime(0);

    while (!glfwWindowShouldClose(window)) {
        float deltaTime = glfwGetTime(); glfwSetTime(0);
        camPitch += speed_x * deltaTime; camYaw += speed_y * deltaTime;
        if (camPitch >  1.40f) camPitch =  1.40f;   // ogranicz pochylenie kamery
        if (camPitch < -1.40f) camPitch = -1.40f;
        fov += speed_zoom * deltaTime;
        if (fov < 10.0f) fov = 10.0f; if (fov > 120.0f) fov = 120.0f;

        if (keyLightUp)   lightIntensity += deltaTime * 0.8f;
        if (keyLightDown) lightIntensity -= deltaTime * 0.8f;
        if (lightIntensity < 0.05f) lightIntensity = 0.05f;
        if (lightIntensity > 3.0f)  lightIntensity = 3.0f;
        if (keyHue) hueOffset += deltaTime * 60.0f;

        for (Fish* fish : aquariumFishes) fish->update(deltaTime);
        for (Plant* plant : aquariumPlants) plant->update(deltaTime);
        appTime += deltaTime;
        bubbles->update(deltaTime);

        drawScene(window, camPitch, camYaw, fov);
        glfwPollEvents();
    }
    freeOpenGLProgram(window); glfwDestroyWindow(window); glfwTerminate(); exit(EXIT_SUCCESS);
}
