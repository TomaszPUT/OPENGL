#include "Plant.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Plant::Plant(glm::vec3 basePos, float height, float swayStrength)
    : basePosition(basePos), totalHeight(height),
    swayStrength(swayStrength), timeAccumulator(0.0f) {
    initGeometry();
}

Plant::~Plant() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Plant::initGeometry() {
    // Jeden segment to prosty prostokąt (quad) - 2 trójkąty
    // Segment: szerokość 0.05, wysokość 1.0 (skalowana później)
    // Układ: X, Y, Z,  U, V,  NX, NY, NZ
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

    // Pozycja
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Normalna
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void*)(5 * sizeof(float)));
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

    float segmentHeight = totalHeight / SEGMENT_COUNT;
    glBindVertexArray(VAO);

    for (int i = 0; i < SEGMENT_COUNT; i++) {
        float ratio = (float)i / (float)SEGMENT_COUNT;

        // Kołysanie rośnie ku górze segmentu
        float sway = swayStrength * ratio
            * sinf(timeAccumulator * 1.5f + ratio * 2.0f);

        // Baza transformacji dla tego segmentu (bez finalnego skalowania)
        glm::mat4 segBase = aquariumBaseM;
        segBase = glm::translate(segBase, basePosition);
        segBase = glm::translate(segBase,
            glm::vec3(0.0f, i * segmentHeight, 0.0f));
        segBase = glm::rotate(segBase, sway, glm::vec3(0.0f, 0.0f, 1.0f));

        // Szerokość segmentu — zwęża się ku górze
        // KLUCZOWE: było 0.12f → teraz 2.5f (20x szerzej!)
        float wScale = (1.0f - ratio * 0.5f) * 2.5f;
        glm::vec3 sv(wScale, segmentHeight, 0.05f);

        // Kolor: ciemnozielony u dołu → jasnozielony u góry
        float g = 0.55f + ratio * 0.40f;
        glUniform4f(glGetUniformLocation(shaderProgram, "objectColor"),
            0.02f, g, 0.05f, 0.92f);

        // ── 4 LIŚCIE na krzyż — widoczne ze wszystkich stron ──

        // Liść 1: twarzą ku kamerze (0°)
        glm::mat4 M1 = glm::scale(segBase, sv);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"),
            1, GL_FALSE, glm::value_ptr(M1));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // Liść 2: obrócony 90° wokół Y
        glm::mat4 M2 = glm::rotate(segBase,
            glm::radians(90.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        M2 = glm::scale(M2, sv);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"),
            1, GL_FALSE, glm::value_ptr(M2));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // Liść 3: odwrócony (180°) — druga strona liścia 1
        glm::mat4 M3 = glm::rotate(segBase,
            glm::radians(180.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        M3 = glm::scale(M3, sv);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"),
            1, GL_FALSE, glm::value_ptr(M3));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // Liść 4: 270° — druga strona liścia 2
        glm::mat4 M4 = glm::rotate(segBase,
            glm::radians(270.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        M4 = glm::scale(M4, sv);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"),
            1, GL_FALSE, glm::value_ptr(M4));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralPlant"), 0);
}   