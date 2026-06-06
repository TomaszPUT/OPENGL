#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Plant {
public:
    Plant(glm::vec3 basePos, float height, float swayStrength);
    ~Plant();

    void update(float deltaTime);
    void draw(GLuint shaderProgram, glm::mat4 aquariumBaseM);

private:
    glm::vec3 basePosition;
    float totalHeight;
    float swayStrength;
    float timeAccumulator;

    float     phase;   // wlasne przesuniecie fazy - kazda roslina kolysze sie inaczej
    glm::vec3 tint;    // wlasny odcien zieleni - urozmaicenie

    static const int SEGMENT_COUNT = 8;

    GLuint VAO, VBO, EBO;
    int indexCount;

    void initGeometry();
};
