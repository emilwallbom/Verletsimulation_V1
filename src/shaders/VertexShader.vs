#version 450 core
layout(location = 0) in vec2 position;

void main()
{

	gl_Position.xy = position.xy*0.001f; 
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
};


