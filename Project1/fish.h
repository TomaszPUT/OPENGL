#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Fish {
public:
    Fish(glm::vec3 startPos, glm::vec3 startColor, glm::vec3 startVelocity);
    ~Fish();

    void update(float deltaTime);
    void draw(GLuint shaderProgram, glm::mat4 aquariumBaseM);

private:
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 velocity;

    GLuint sphereVAO, sphereVBO, sphereEBO;
    int sphereIndexCount;
    GLuint tailVAO, tailVBO;

    void initSphere(float radius, int sectors, int stacks);
    void initTail();
};
