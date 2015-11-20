#version 330 

layout (location = 0) in vec3 vertPos; 
uniform mat4 viewMatrix, projMatrix; 
uniform vec3 colorIn; 
out vec3 color;

void main(){ 	
	color = colorIn;		
	gl_Position =  projMatrix * viewMatrix * vec4(vertPos, 1.0); 
}
