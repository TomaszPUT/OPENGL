#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

// ============================================================
//  Model - wczytuje siatke 3D z pliku .obj (format Wavefront).
//  Czytamy tylko to, czego potrzebujemy:
//    v  = pozycja wierzcholka (x y z)
//    vt = wspolrzedne tekstury (u v)
//    vn = normalna (nx ny nz)
//    f  = sciana jako trojkat: "f w1/t1/n1 w2/t2/n2 w3/t3/n3"
//  Z tego skladamy JEDNA plaska tablice [x,y,z,u,v,nx,ny,nz]
//  i rysujemy ja przez glDrawArrays - dokladnie tak samo jak
//  reszta naszych obiektow. Caly kod rysujacy jest nasz wlasny.
// ============================================================
class Model {
public:
    Model(const char* filename);
    ~Model();

    // Ustawia macierz M i rysuje model (caller podaje gotowa macierz)
    void draw(GLuint shaderProgram, glm::mat4 M);

private:
    GLuint VAO, VBO;
    int    vertexCount;   // liczba wierzcholkow do narysowania
    bool   loaded;

    void load(const char* filename);
};
