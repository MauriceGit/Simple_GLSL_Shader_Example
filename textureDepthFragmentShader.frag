#version 330

in vec2 texPos;
uniform sampler2D texsampler;
out vec4 Color;

float linearizeDepth (float depth) {
    float nearPlane = 0.5, farPlane = 50.0;
    return (2*nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

//
// Ganz normale Ansicht der jew. Tiefe.
//
void main(){
  Color = vec4(linearizeDepth(texture(texsampler, texPos).x));
}

