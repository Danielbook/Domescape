#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 inPositions;
layout(location = 1) in vec2 inCoords;
layout(location = 2) in vec3 inNormals;

uniform mat4 MVP; //Model View Projection Matrix
uniform mat3 NM; //Normal Matrix

out vec2 UV;
out vec3 tNormals; //transformed normals

void main()
{
    // Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(inPositions * vec3(0.0f, 0.0f, 0.0f), 1.0);
	UV = inCoords;
	tNormals = normalize(NM * inNormals);
}
