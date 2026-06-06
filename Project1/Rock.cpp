#include "Rock.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    int sectors = 20; int stacks = 20; float radius = 1.0f;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stacks;
        float y = radius * sinf(stackAngle);
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectors;
            float x = radius * cosf(stackAngle) * cosf(sectorAngle);
            float z = radius * cosf(stackAngle) * sinf(sectorAngle);

            float u = (float)j / sectors; float v = (float)i / stacks;
            float nx = x / radius; float ny = y / radius; float nz = z / radius; // Matematyczna normalna kuli

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
    // Obliczanie pozycji i deformacji kamienia
    glm::mat4 M = aquariumBaseM;
    M = glm::translate(M, position);
    M = glm::scale(M, scale); // To "spłaszcza" sferę w płaski kamień!

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"), 1, GL_FALSE, glm::value_ptr(M));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}