#include "Rock.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>

Rock::Rock(glm::vec3 startPos, glm::vec3 startScale) : position(startPos), scale(startScale) {
    initGeometry();
}

Rock::~Rock() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Rock::initGeometry() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int sectors = 24; int stacks = 24; float radius = 1.0f;

    // Losowe fazy - dzieki nim KAZDY kamien ma inny ksztalt.
    // (srand() jest wywolane raz w main, wiec za kazdym razem inne wartosci.)
    float ph1 = (float)rand() / RAND_MAX * 6.28f;
    float ph2 = (float)rand() / RAND_MAX * 6.28f;
    float ph3 = (float)rand() / RAND_MAX * 6.28f;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stacks;
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectors;

            // ── Nieregularny promien: kula + kilka fal = bryla glazu ──
            // Niskie czestotliwosci = duze garby, wyzsze = drobne nierownosci.
            float bump =
                  0.22f * sinf(3.0f * sectorAngle + ph1) * cosf(2.0f * stackAngle + ph2)
                + 0.12f * sinf(5.0f * stackAngle + ph3)
                + 0.07f * cosf(7.0f * sectorAngle + ph2);
            float r = radius * (1.0f + bump);

            float x = r * cosf(stackAngle) * cosf(sectorAngle);
            float y = r * sinf(stackAngle);
            float z = r * cosf(stackAngle) * sinf(sectorAngle);

            float u = (float)j / sectors; float v = (float)i / stacks;
            // Normalna przyblizona kierunkiem od srodka (wystarcza dla glazu)
            float len = sqrtf(x * x + y * y + z * z);
            float nx = x / len; float ny = y / len; float nz = z / len;

            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
            vertices.push_back(u); vertices.push_back(v);
            vertices.push_back(nx); vertices.push_back(ny); vertices.push_back(nz);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            unsigned int k1 = i * (sectors + 1) + j; unsigned int k2 = k1 + sectors + 1;
            indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1);
            indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1);
        }
    }
    indexCount = indices.size();

    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Rock::draw(GLuint shaderProgram, glm::mat4 aquariumBaseM) {
    glm::mat4 M = aquariumBaseM;
    M = glm::translate(M, position);
    M = glm::scale(M, scale); // splaszcza glaz, zeby lezal na dnie

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"), 1, GL_FALSE, glm::value_ptr(M));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
