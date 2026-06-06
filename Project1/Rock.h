#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Rock {
public:
    // Konstruktor przyjmuje pozycję i zniekształcenie (skalę)
    Rock(glm::vec3 startPos, glm::vec3 startScale);
    ~Rock();
    void draw(GLuint shaderProgram, glm::mat4 aquariumBaseM);

private:
    glm::vec3 position;
    glm::vec3 scale;

    GLuint VAO, VBO, EBO;
    int indexCount;

    void initGeometry();
};