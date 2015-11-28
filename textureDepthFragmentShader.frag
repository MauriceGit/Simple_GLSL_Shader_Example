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

void main(){
	Color = vec4(linearizeDepth(texture(texsampler, texPos).x)); 
}
