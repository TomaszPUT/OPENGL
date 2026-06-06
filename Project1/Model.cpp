#define _CRT_SECURE_NO_WARNINGS
#include "Model.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

Model::Model(const char* filename) : VAO(0), VBO(0), vertexCount(0), loaded(false) {
    load(filename);
}

Model::~Model() {
    if (loaded) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

// Pomocnik: z tokenu "v/t/n", "v//n", "v/t" lub "v" wyciaga indeksy (0-based).
// Obsluguje tez indeksy ujemne (liczone od konca), ktore zdarzaja sie w plikach z netu.
static void parseRef(const std::string& tok, int posCount, int texCount, int norCount,
                     int& pi, int& ti, int& ni) {
    pi = ti = ni = -1;
    int part = 0; std::string cur;
    for (size_t k = 0; k <= tok.size(); ++k) {
        if (k == tok.size() || tok[k] == '/') {
            if (!cur.empty()) {
                int val = atoi(cur.c_str());
                if (part == 0) pi = (val > 0) ? val - 1 : posCount + val;
                if (part == 1) ti = (val > 0) ? val - 1 : texCount + val;
                if (part == 2) ni = (val > 0) ? val - 1 : norCount + val;
            }
            cur.clear(); part++;
        } else cur += tok[k];
    }
}

void Model::load(const char* filename) {
    std::vector<glm::vec3> pos;
    std::vector<glm::vec2> tex;
    std::vector<glm::vec3> nor;
    std::vector<float>     data;   // gotowa tablica [x,y,z,u,v,nx,ny,nz] * wierzcholki

    std::ifstream in(filename);
    if (!in.is_open()) { printf("Model: nie moge otworzyc %s\n", filename); return; }

    std::string line;
    while (std::getline(in, line)) {
        if (line.size() < 2) continue;
        std::istringstream ss(line);
        std::string tag; ss >> tag;

        if (tag == "v") {
            glm::vec3 p; ss >> p.x >> p.y >> p.z; pos.push_back(p);
        } else if (tag == "vt") {
            glm::vec2 t; ss >> t.x >> t.y; tex.push_back(t);
        } else if (tag == "vn") {
            glm::vec3 n; ss >> n.x >> n.y >> n.z; nor.push_back(n);
        } else if (tag == "f") {
            // Zbierz wszystkie narozniki sciany (moze byc trojkat, czworokat lub n-kat)
            std::vector<int> fp, ft, fn; std::string tok;
            while (ss >> tok) {
                int pi, ti, ni;
                parseRef(tok, (int)pos.size(), (int)tex.size(), (int)nor.size(), pi, ti, ni);
                fp.push_back(pi); ft.push_back(ti); fn.push_back(ni);
            }
            // Triangulacja "wachlarzem": (0,1,2),(0,2,3),...
            for (size_t i = 1; i + 1 < fp.size(); ++i) {
                int idx[3] = { 0, (int)i, (int)i + 1 };
                // jesli brak normalnych w pliku - policz wlasna z trojkata
                glm::vec3 gn(0.0f, 1.0f, 0.0f);
                if (fn[idx[0]] < 0 || fn[idx[1]] < 0 || fn[idx[2]] < 0) {
                    glm::vec3 a = pos[fp[idx[0]]], b = pos[fp[idx[1]]], c = pos[fp[idx[2]]];
                    gn = glm::normalize(glm::cross(b - a, c - a));
                }
                for (int k = 0; k < 3; ++k) {
                    glm::vec3 p = pos[fp[idx[k]]];
                    glm::vec2 t = (ft[idx[k]] >= 0 && ft[idx[k]] < (int)tex.size()) ? tex[ft[idx[k]]] : glm::vec2(0.0f);
                    glm::vec3 n = (fn[idx[k]] >= 0 && fn[idx[k]] < (int)nor.size()) ? nor[fn[idx[k]]] : gn;
                    data.push_back(p.x); data.push_back(p.y); data.push_back(p.z);
                    data.push_back(t.x); data.push_back(t.y);
                    data.push_back(n.x); data.push_back(n.y); data.push_back(n.z);
                }
            }
        }
    }
    in.close();

    if (pos.empty() || data.empty()) { printf("Model %s: pusty/niezrozumialy plik\n", filename); return; }

    // ── AUTO-NORMALIZACJA ──
    // Wysrodkowujemy model w X i Z, stawiamy jego SPOD na y=0 (zeby stal na dnie,
    // a nie wisial), i skalujemy tak, by najwiekszy wymiar = 1. Dzieki temu DOWOLNY
    // model z internetu po prostu staje na piasku po ustawieniu pozycji w main.
    glm::vec3 mn = pos[0], mx = pos[0];
    for (auto& p : pos) { mn = glm::min(mn, p); mx = glm::max(mx, p); }
    glm::vec3 size = mx - mn;
    float maxDim = glm::max(size.x, glm::max(size.y, size.z));
    if (maxDim <= 0.0f) maxDim = 1.0f;
    glm::vec3 shift((mn.x + mx.x) * 0.5f, mn.y, (mn.z + mx.z) * 0.5f); // spod na 0
    for (size_t i = 0; i < data.size(); i += 8) {
        data[i + 0] = (data[i + 0] - shift.x) / maxDim;
        data[i + 1] = (data[i + 1] - shift.y) / maxDim;
        data[i + 2] = (data[i + 2] - shift.z) / maxDim;
    }

    vertexCount = (int)(data.size() / 8);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    loaded = true;
    printf("Model %s: %d wierzcholkow (wysrodkowany, spod na y=0, skala 1.0)\n", filename, vertexCount);
}

void Model::draw(GLuint shaderProgram, glm::mat4 M) {
    if (!loaded) return;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"), 1, GL_FALSE, glm::value_ptr(M));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
