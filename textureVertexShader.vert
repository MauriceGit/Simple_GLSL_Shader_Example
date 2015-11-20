#version 330 

layout (location = 0) in vec3 vertPos; 
layout (location = 1) in vec2 texPosIn;
uniform mat4 viewMatrix, projMatrix; 
out vec2 texPos;

void main(){ 	
	texPos = texPosIn;	
	gl_Position =  projMatrix * viewMatrix * vec4(vertPos, 1.0); 
}
