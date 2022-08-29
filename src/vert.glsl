#version 330 core

layout(location = 1) in vec2 VERT_IN_POSITION;

void main() {
    gl_Position = vec4(VERT_IN_POSITION, 0.0f, 1.0f);
}
