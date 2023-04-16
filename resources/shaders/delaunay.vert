#version 330 core

layout(location = 0) in float id;
layout(location = 1) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 outColor;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.);
    outColor = vec4(1., 0., 0., 1.);
}