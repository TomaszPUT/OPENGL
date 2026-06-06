#include "Bubbles.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>

static float frand(float a, float b) { return a + (float)rand() / RAND_MAX * (b - a); }

Bubbles::Bubbles(int count) : timeAcc(0.0f) {
    initSphere();
    for (int i = 0; i < count; i++) {
        Bubble b;
        respawn(b);
        b.pos.y = frand(-1.9f, 1.4f);   // na starcie rozrzucone na roznych wysokosciach
        bubbles.push_back(b);
    }
}

Bubbles::~Bubbles() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Bubbles::respawn(Bubble& b) {
    b.pos    = glm::vec3(frand(-2.6f, 2.6f), -1.9f, frand(-1.6f, 1.6f));
    b.speed  = frand(0.15f, 0.45f);   // wolniej -> spokojniejsze, bardziej zywe
    b.size   = frand(0.04f, 0.13f);
    b.wobble = frand(0.04f, 0.15f);
    b.phase  = frand(0.0f, 6.28f);
    b.phaseZ = frand(0.0f, 6.28f);
    b.alpha  = frand(0.0f, 0.30f);
}

void Bubbles::initSphere() {
    // Mala, niskorozdzielcza kula (wystarczy dla babelka)
    std::vector<float> v; std::vector<unsigned int> idx;
    int sectors = 18, stacks = 14; float r = 1.0f;   // wieksza rozdzielczosc = okragly babelek
    for (int i = 0; i <= stacks; ++i) {
        float sa = glm::pi<float>() / 2 - i * glm::pi<float>() / stacks;
        float y = r * sinf(sa);
        for (int j = 0; j <= sectors; ++j) {
            float se = j * 2 * glm::pi<float>() / sectors;
            float x = r * cosf(sa) * cosf(se);
            float z = r * cosf(sa) * sinf(se);
            v.push_back(x); v.push_back(y); v.push_back(z);
            v.push_back((float)j / sectors); v.push_back((float)i / stacks);
            v.push_back(x); v.push_back(y); v.push_back(z); // normalna = pozycja (kula r=1)
        }
    }
    for (int i = 0; i < stacks; ++i)
        for (int j = 0; j < sectors; ++j) {
            unsigned int k1 = i * (sectors + 1) + j, k2 = k1 + sectors + 1;
            idx.push_back(k1); idx.push_back(k2); idx.push_back(k1 + 1);
            idx.push_back(k1 + 1); idx.push_back(k2); idx.push_back(k2 + 1);
        }
    indexCount = (int)idx.size();

    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Bubbles::update(float deltaTime) {
    timeAcc += deltaTime;
    for (Bubble& b : bubbles) {
        // lekkie przyspieszanie przy wznoszeniu (jak prawdziwe babelki)
        b.pos.y += b.speed * (1.0f + 0.25f * (b.pos.y + 1.9f)) * deltaTime;
        if (b.pos.y > 1.45f) respawn(b);
    }
}

void Bubbles::draw(GLuint shaderProgram, glm::mat4 aquariumBaseM) {
    glUniform1i(glGetUniformLocation(shaderProgram, "isBubble"), 1);
    GLint locC = glGetUniformLocation(shaderProgram, "objectColor");
    glBindVertexArray(VAO);
    for (Bubble& b : bubbles) {
        // delikatny, wolny dryf w DWoch osiach -> ruch wyglada zywo, nie plasko
        float ox = sinf(timeAcc * 1.0f + b.phase)  * b.wobble;
        float oz = sinf(timeAcc * 0.8f + b.phaseZ) * b.wobble * 0.8f;
        glm::mat4 M = aquariumBaseM;
        M = glm::translate(M, b.pos + glm::vec3(ox, 0.0f, oz));
        M = glm::scale(M, glm::vec3(b.size));
        glUniform4f(locC, 0.8f, 0.9f, 1.0f, b.alpha);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"), 1, GL_FALSE, glm::value_ptr(M));
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
    glUniform1i(glGetUniformLocation(shaderProgram, "isBubble"), 0);
}
