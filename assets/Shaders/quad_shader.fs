#version 330 core

out vec4 FragColour;
in vec2 TexCoords;

uniform sampler2D ScreenTexture;
uniform float time;

void main() {
    // /FragColour = texture(ScreenTexture, TexCoords + 0.005*vec2( sin(time+1024.0*TexCoords.x), cos(time+768.0*TexCoords.y)) );
    //FragColour = texture(ScreenTexture, TexCoords);
}
