#include "Cube.h"

Cube::Cube() {
    // Układ: X, Y, Z,   U, V,   NX, NY, NZ
    float vertices[] = {
        -3.0f, -2.0f,  2.0f,   0.0f, 0.0f,  -0.6f, -0.4f,  0.6f,
         3.0f, -2.0f,  2.0f,   1.0f, 0.0f,   0.6f, -0.4f,  0.6f,
         3.0f,  2.0f,  2.0f,   1.0f, 1.0f,   0.6f,  0.4f,  0.6f,
        -3.0f,  2.0f,  2.0f,   0.0f, 1.0f,  -0.6f,  0.4f,  0.6f,
        -3.0f, -2.0f, -2.0f,   1.0f, 0.0f,  -0.6f, -0.4f, -0.6f,
         3.0f, -2.0f, -2.0f,   0.0f, 0.0f,   0.6f, -0.4f, -0.6f,
         3.0f,  2.0f, -2.0f,   0.0f, 1.0f,   0.6f,  0.4f, -0.6f,
        -3.0f,  2.0f, -2.0f,   1.0f, 1.0f,  -0.6f,  0.4f, -0.6f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,  1, 5, 6, 6, 2, 1,
        5, 4, 7, 7, 6, 5,  4, 0, 3, 3, 7, 4,
        4, 5, 1, 1, 0, 4
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Atrybut 0: Pozycja (X, Y, Z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atrybut 1: Tekstura (U, V)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Atrybut 2: Normalna (NX, NY, NZ)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}
Cube::~Cube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Cube::drawSolid() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cube::drawEdges() {
    unsigned int edgeIndices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    GLuint edgeEBO;
    glGenBuffers(1, &edgeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edgeIndices), edgeIndices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &edgeEBO);
}