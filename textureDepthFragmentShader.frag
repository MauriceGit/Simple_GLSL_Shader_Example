#version 330

in vec2 texPos;
uniform sampler2D texsampler;
out vec4 Color;

float linearizeDepth (float depth) {
    float nearPlane = 0.5, farPlane = 50.0;
    return (2*nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

float linearizeDepthExtreme (float depth) {
    float nearPlane = 0.5, farPlane = 50.0;
    float res = (2*nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
    res = (res+0.3)*(res+0.3) - 0.5;
    return res;
}

float colorDiff(float d1, float d2) {
    return abs(d1 - d2);
}

float getTexValue(vec2 texPos, float xDiff, float yDiff) {
    return linearizeDepth(texture(texsampler, vec2(texPos.x+xDiff, texPos.y+yDiff)).x);
}

vec3 calcColor(vec3 baseColor, vec2 texPos) {
    float pixelDiffX = 1.0 / 1920.0;
    float pixelDiffY = 1.0 / 1080.0;
    float thisPixel = linearizeDepth(texture(texsampler, texPos).x);
    float diff = 0.0;

    //diff += colorDiff(thisPixel, getTexValue(texPos, -pixelDiffX, 0));
    //diff += colorDiff(thisPixel, getTexValue(texPos, +pixelDiffX, 0));
    //diff += colorDiff(thisPixel, getTexValue(texPos, 0, -pixelDiffY));
    //diff += colorDiff(thisPixel, getTexValue(texPos, 0, +pixelDiffY));

    int range = 6;

    for (int i = -range; i < range; i++) {
        for (int j = -range; j < range; j++) {
            diff += colorDiff(thisPixel, getTexValue(texPos, i*pixelDiffX, j*pixelDiffY));
        }
    }



    return mix(vec3(1.0), vec3(0.0), diff/100);

}

//
// Ganz normale Ansicht der jew. Tiefe.
//
//void main(){
//  Color = vec4(linearizeDepth(texture(texsampler, texPos).x));
//}

//
// Hier mit Kantendetektion die Pixel um das aktuelle rum.
//
void main() {
    Color = vec4(calcColor(vec3(1.0, 1.0, 1.0), texPos), 1.0);
}
