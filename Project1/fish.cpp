#include "Fish.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Fish::Fish(glm::vec3 startPos, glm::vec3 startColor, glm::vec3 startVelocity)
    : position(startPos), color(startColor), velocity(startVelocity) {
    initSphere(1.0f, 20, 20);
    initTail();
}

Fish::~Fish() {
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &tailVAO);
}

void Fish::update(float deltaTime) {
    position += velocity * deltaTime;

    float paddingX = 0.4f;
    float paddingY = 0.4f;
    float paddingZ = 0.4f;

    float maxX = 2.8f - paddingX;
    float minX = -2.8f + paddingX;
    float maxY = 1.5f - paddingY;
    float minY = -1.8f + paddingY;
    float maxZ = 1.8f - paddingZ;
    float minZ = -1.8f + paddingZ;

    if (position.x > maxX) { position.x = maxX;  velocity.x = -velocity.x; }
    if (position.x < minX) { position.x = minX;  velocity.x = -velocity.x; }

    if (position.y > maxY) { position.y = maxY;  velocity.y = -velocity.y; }
    if (position.y < minY) { position.y = minY;  velocity.y = -velocity.y; }

    if (position.z > maxZ) { position.z = maxZ;  velocity.z = -velocity.z; }
    if (position.z < minZ) { position.z = minZ;  velocity.z = -velocity.z; }
}
void Fish::initSphere(float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stacks;
        float y = radius * sinf(stackAngle);
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectors;
            float x = radius * cosf(stackAngle) * cosf(sectorAngle);
            float z = radius * cosf(stackAngle) * sinf(sectorAngle);
            float u = (float)j / sectors; float v = (float)i / stacks;
            float nx = x / radius; float ny = y / radius; float nz = z / radius;

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
    sphereIndexCount = indices.size();

    glGenVertexArrays(1, &sphereVAO); glGenBuffers(1, &sphereVBO); glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Fish::initTail() {
    float tailVertices[] = {
        0.0f,  0.0f,  0.0f,  0.5f, 0.5f,  1.0f,  0.0f,  0.0f,
       -1.0f,  0.8f,  0.5f,  0.0f, 1.0f, -1.0f,  0.5f,  0.5f,
       -1.0f, -0.8f,  0.5f,  0.0f, 0.0f, -1.0f, -0.5f,  0.5f,
       -1.0f, -0.8f, -0.5f,  1.0f, 0.0f, -1.0f, -0.5f, -0.5f,
       -1.0f,  0.8f, -0.5f,  1.0f, 1.0f, -1.0f,  0.5f, -0.5f
    };
    unsigned int tailIndices[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,1, 1,2,3, 3,4,1 };

    GLuint tailEBO;
    glGenVertexArrays(1, &tailVAO); glGenBuffers(1, &tailVBO); glGenBuffers(1, &tailEBO);
    glBindVertexArray(tailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tailVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tailVertices), tailVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tailEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tailIndices), tailIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Fish::draw(GLuint shaderProgram, glm::mat4 aquariumBaseM) {
    glUseProgram(shaderProgram);

    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralFish"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralPlant"), 0);

    float fishYaw = atan2(-velocity.z, velocity.x);

    glm::mat4 baseM = aquariumBaseM;
    baseM = glm::translate(baseM, position);
    baseM = glm::rotate(baseM, fishYaw, glm::vec3(0, 1, 0));

    glm::mat4 MBody = baseM;
    MBody = glm::scale(MBody, glm::vec3(0.25f, 0.20f, 0.15f));
    glUniform4f(glGetUniformLocation(shaderProgram, "objectColor"),
        color.r, color.g, color.b, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"),
        1, GL_FALSE, glm::value_ptr(MBody));
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

    glm::mat4 MTail = baseM;
    MTail = glm::translate(MTail, glm::vec3(-0.2f, 0.0f, 0.0f));
    MTail = glm::scale(MTail, glm::vec3(0.25f, 0.25f, 0.25f));
    glUniform4f(glGetUniformLocation(shaderProgram, "objectColor"),
        color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"),
        1, GL_FALSE, glm::value_ptr(MTail));
    glBindVertexArray(tailVAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "useProceduralFish"), 0);
}
