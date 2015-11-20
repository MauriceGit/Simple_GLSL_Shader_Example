#version 330 

in vec2 texPos;
out vec4 Color; 
uniform sampler2D texsampler;


void main(){ 
	Color = texture(texsampler, texPos); 
}
