#version 330 core

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

out vec2 iTexCoord;
out vec3 iNormal;
out vec3 iFragPos;

void main(void) {
    gl_Position = P * V * M * vec4(vertex, 1.0);

    iTexCoord = texCoord;
    iFragPos  = vec3(M * vec4(vertex, 1.0));
    iNormal   = mat3(transpose(inverse(M))) * normal;
}