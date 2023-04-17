#version 330 core

uniform sampler2D ourTexture;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 lightPosition;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

void main() {

    vec3 color = vec3(texture(ourTexture, texCoord));

    vec3 ambient = color * ambient;
    vec3 lightDir = normalize(lightPosition - fragPos);
    vec3 diffuse = color * diffuse * (max(dot(normal, lightDir), 0.));

    gl_FragColor = vec4(ambient + diffuse, 1.);
}