#version 330 core


out vec4 FragColor;
in vec3 outNormal;
uniform sampler2D ourTexture;

void main() {
   FragColor = vec4(outNormal, 1.0);
}
