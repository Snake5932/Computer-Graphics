#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 rModel;
uniform mat4 lookAt;
uniform mat4 projection;

void main() {
    gl_Position = projection * lookAt * rModel * model * vec4(position, 1.0f);
}