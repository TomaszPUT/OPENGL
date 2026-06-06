#pragma once
#include <GL/glew.h>

class Cube {
public:
    Cube();
    ~Cube();
    void drawSolid();   // pelna bryla - teraz WSZYSTKIE 6 scian (z gornym dnem!)
    void drawEdges();   // sam szkielet (12 krawedzi)

private:
    GLuint solidVAO, solidVBO, solidEBO;
    GLuint edgeVAO, edgeVBO, edgeEBO;
};
