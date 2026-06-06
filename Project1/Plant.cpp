#include "Plant.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>

Plant::Plant(glm::vec3 basePos, float height, float swayStrength)
    : basePosition(basePos), totalHeight(height),
    swayStrength(swayStrength), timeAccumulator(0.0f) {

    phase = (float)rand() / RAND_MAX * 6.28f;

    float k = (float)rand() / RAND_MAX;
    if (k < 0.6f)       tint = glm::vec3(0.06f, 0.50f, 0.16f);  // zwykla zielen
    else if (k < 0.85f) tint = glm::vec3(0.10f, 0.46f, 0.34f);  // seledyn / morska
    else                tint = glm::vec3(0.26f, 0.46f, 0.10f);  // oliwkowa

    initGeometry();
}

Plant::~Plant() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Plant::initGeometry() {
    float segVertices[] = {
        -0.05f, 0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.05f, 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.05f, 1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.05f, 1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    };
    unsigned int segIndices[] = { 0, 1, 2,  0, 2, 3 };
    indexCount = 6;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(segVertices), segVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(segIndices), segIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Plant::update(float deltaTime) {
    timeAccumulator += deltaTime;
}

void Plant::draw(GLuint shaderProgram, glm::mat4 aquariumBaseM) {
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralFish"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralPlant"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "isRock"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "isEmissive"), 0);

    GLint locM = glGetUniformLocation(shaderProgram, "M");
    GLint locC = glGetUniformLocation(shaderProgram, "objectColor");
    glBindVertexArray(VAO);

    // KEPA: kilka lodyg blisko siebie -> bujny krzaczek (nie jedna mizerna lodyga)
    const int STALKS = 5;
    const float ox[STALKS] = { 0.00f, 0.16f, -0.14f, 0.08f, -0.09f };
    const float oz[STALKS] = { 0.00f, -0.12f, 0.10f, 0.14f, -0.13f };
    const float oh[STALKS] = { 1.00f, 0.78f, 0.85f, 0.66f, 0.72f }; // rozne wysokosci

    for (int st = 0; st < STALKS; st++) {
        float stalkHeight = totalHeight * oh[st];
        float segmentHeight = stalkHeight / SEGMENT_COUNT;
        float stalkPhase = phase + st * 1.1f;
        glm::vec3 stalkBase = basePosition + glm::vec3(ox[st], 0.0f, oz[st]);

        for (int i = 0; i < SEGMENT_COUNT; i++) {
            float ratio = (float)i / (float)SEGMENT_COUNT;

            float t     = timeAccumulator * 1.5f + stalkPhase;
            float swayZ = swayStrength * ratio * sinf(t + ratio * 2.0f);
            float swayX = swayStrength * 0.5f * ratio * sinf(t * 0.7f + ratio * 1.3f);

            glm::mat4 segBase = aquariumBaseM;
            segBase = glm::translate(segBase, stalkBase);
            segBase = glm::translate(segBase, glm::vec3(0.0f, i * segmentHeight, 0.0f));
            segBase = glm::rotate(segBase, swayZ, glm::vec3(0.0f, 0.0f, 1.0f));
            segBase = glm::rotate(segBase, swayX, glm::vec3(1.0f, 0.0f, 0.0f));

            // Wstega: w miare szeroka, lekko zwezona ku gorze -> bujny lisc
            float wScale = (1.0f - ratio * 0.45f) * 1.5f + 0.18f;
            glm::vec3 sv(wScale, segmentHeight, 0.05f);

            glm::vec3 c = tint * (0.55f + ratio * 0.6f);
            glUniform4f(locC, c.r, c.g, c.b, 1.0f);

            // 2 skrzyzowane liscie (0 i 90 stopni). Bez cullingu kazdy widac z obu
            // stron, wiec nie trzeba 4 - a 0 i 180 to ta sama plaszczyzna (migotala).
            for (int rot = 0; rot < 2; rot++) {
                glm::mat4 M = glm::rotate(segBase,
                    glm::radians(90.0f * rot), glm::vec3(0.0f, 1.0f, 0.0f));
                M = glm::scale(M, sv);
                glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(M));
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
            }
        }
    }

    glBindVertexArray(0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralPlant"), 0);
}
