#pragma once
#include <GL/glew.h>

class Cube {
public:
    Cube();
    ~Cube();
    void drawSolid(); // Nazwa funkcji dostosowana do szablonu z Twoich zajęć (Models::torus.drawSolid())
    void drawEdges();

private:
    GLuint VAO, VBO, EBO;
};