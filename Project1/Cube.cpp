#include "Cube.h"

// Pudelko x[-3,3] y[-2,2] z[-2,2]. Kazda sciana ma 4 wlasne wierzcholki,
// dzieki czemu kazda ma poprawne UV (0..1) i wlasna, prostopadla normalna.
Cube::Cube() {
    // Format: X,Y,Z,  U,V,  NX,NY,NZ
    float v[] = {
        // PRZOD (z = +2), normalna (0,0,1)
        -3,-2, 2,  0,0,  0,0,1,
         3,-2, 2,  1,0,  0,0,1,
         3, 2, 2,  1,1,  0,0,1,
        -3, 2, 2,  0,1,  0,0,1,
        // TYL (z = -2), normalna (0,0,-1)
         3,-2,-2,  0,0,  0,0,-1,
        -3,-2,-2,  1,0,  0,0,-1,
        -3, 2,-2,  1,1,  0,0,-1,
         3, 2,-2,  0,1,  0,0,-1,
        // LEWA (x = -3), normalna (-1,0,0)
        -3,-2,-2,  0,0, -1,0,0,
        -3,-2, 2,  1,0, -1,0,0,
        -3, 2, 2,  1,1, -1,0,0,
        -3, 2,-2,  0,1, -1,0,0,
        // PRAWA (x = +3), normalna (1,0,0)
         3,-2, 2,  0,0,  1,0,0,
         3,-2,-2,  1,0,  1,0,0,
         3, 2,-2,  1,1,  1,0,0,
         3, 2, 2,  0,1,  1,0,0,
        // GORA (y = +2), normalna (0,1,0)
        -3, 2, 2,  0,0,  0,1,0,
         3, 2, 2,  1,0,  0,1,0,
         3, 2,-2,  1,1,  0,1,0,
        -3, 2,-2,  0,1,  0,1,0,
        // DOL (y = -2), normalna (0,-1,0)
        -3,-2,-2,  0,0,  0,-1,0,
         3,-2,-2,  1,0,  0,-1,0,
         3,-2, 2,  1,1,  0,-1,0,
        -3,-2, 2,  0,1,  0,-1,0,
    };
    unsigned int idx[36];
    for (int f = 0; f < 6; f++) {
        unsigned int b = f * 4;
        idx[f*6+0]=b;   idx[f*6+1]=b+1; idx[f*6+2]=b+2;
        idx[f*6+3]=b;   idx[f*6+4]=b+2; idx[f*6+5]=b+3;
    }

    glGenVertexArrays(1, &solidVAO); glGenBuffers(1, &solidVBO); glGenBuffers(1, &solidEBO);
    glBindVertexArray(solidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, solidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, solidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // --- osobny, prosty bufor na szkielet (8 naroznikow) ---
    float ev[] = {
        -3,-2, 2,  0,0, 0,0,0,   3,-2, 2,  0,0, 0,0,0,
         3, 2, 2,  0,0, 0,0,0,  -3, 2, 2,  0,0, 0,0,0,
        -3,-2,-2,  0,0, 0,0,0,   3,-2,-2,  0,0, 0,0,0,
         3, 2,-2,  0,0, 0,0,0,  -3, 2,-2,  0,0, 0,0,0,
    };
    unsigned int ei[] = {
        0,1, 1,2, 2,3, 3,0,
        4,5, 5,6, 6,7, 7,4,
        0,4, 1,5, 2,6, 3,7
    };
    glGenVertexArrays(1, &edgeVAO); glGenBuffers(1, &edgeVBO); glGenBuffers(1, &edgeEBO);
    glBindVertexArray(edgeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ev), ev, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ei), ei, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

Cube::~Cube() {
    glDeleteVertexArrays(1, &solidVAO); glDeleteBuffers(1, &solidVBO); glDeleteBuffers(1, &solidEBO);
    glDeleteVertexArrays(1, &edgeVAO);  glDeleteBuffers(1, &edgeVBO);  glDeleteBuffers(1, &edgeEBO);
}

void Cube::drawSolid() {
    glBindVertexArray(solidVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cube::drawEdges() {
    glBindVertexArray(edgeVAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
