#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 inPositions;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec2 inCoords;


uniform mat4 MVP; //Model View Projection Matrix
//uniform mat4 P; //Projection  Matrix
uniform mat3 NM; //Normal Matrix
uniform vec3 lightDir; //Calculated light direction

out vec3 lDir;
out vec2 UV;
out vec3 tnormals; //transformed normals

void main()
{
    // Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(inPositions + vec3(0.0, 2.0, 0.0), 1.0);

	UV = inCoords;

	lDir = normalize(NM * lightDir);

	tnormals = normalize(NM * inNormals);

}
