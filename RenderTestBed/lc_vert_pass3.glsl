#version 330

layout(location = 0)in vec3 vertex;
layout(location = 1)in vec3 color;
layout(location = 2)in vec3 normal;
layout(location = 3)in vec3 uv;

out vec3 outUV;

void main(){
	
	outUV=(vertex.xyz*0.5)+vec3(0.5);
	outUV=uv;
	gl_Position = vec4(vertex.xy,0,1.0);

}