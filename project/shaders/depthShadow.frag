#version 330 core

//in vec2 UV;

//out vec4 color;

layout(location = 0) out float fragmentdepth;


void main()
{
	fragmentdepth = gl_FragCoord.z;
}

