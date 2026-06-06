#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

// Strumien babelkow unoszacych sie ku gorze. Jedna mala kula (siatka)
// jest wspoldzielona, a kazdy babelek ma wlasna pozycje, predkosc i
// drobne falowanie w bok. Po dotarciu do powierzchni babelek wraca na dol.
class Bubbles {
public:
    Bubbles(int count);
    ~Bubbles();

    void update(float deltaTime);
    void draw(GLuint shaderProgram, glm::mat4 aquariumBaseM);

private:
    struct Bubble {
        glm::vec3 pos;
        float speed;
        float size;
        float wobble;   // amplituda kolysania w bok
        float phase;    // faza kolysania w X
        float phaseZ;   // faza kolysania w Z (osobna -> ruch nie jest plaski)
        float alpha;    // losowe krycie 0..0.3
    };

    std::vector<Bubble> bubbles;
    float timeAcc;

    GLuint VAO, VBO, EBO;
    int    indexCount;

    void initSphere();
    void respawn(Bubble& b);
};
