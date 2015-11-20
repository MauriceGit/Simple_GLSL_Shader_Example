#version 330 

layout (location = 0) in vec3 vertPos; 
uniform mat4 viewMatrix, projMatrix; 
out vec3 color;

void main(){ 	
	color = (vertPos + 10.0) / 20.0 ;	
	gl_Position =  projMatrix * viewMatrix * vec4(vertPos, 1.0); 
}
