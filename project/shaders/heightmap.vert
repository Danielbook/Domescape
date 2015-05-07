#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertPositions;
layout(location = 1) in vec2 texCoords;

uniform sampler2D hTex;
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 MV_light;
uniform vec3 light_dir;
uniform mat3 normalMatrix;


out vec2 UV; //texture coords
out float vScale; // Height scaling
out vec3 lDir;
out mat3 NM;


void main()
{
    UV = texCoords;

    vScale = 60;
    float hVal = texture( hTex, UV ).r;
	vec4 transformedVertex = vec4(vertPositions + vec3(0.0, hVal * vScale, 0.0), 1.0);

/*	vec3 v = vec3(MV * transformedVertex);
	vec3 l = vec3(mat3(MV_light) * light_dir);
	lDir = normalize(l-v);
*/
	lDir = normalize(normalMatrix * light_dir);

	NM = normalMatrix;


	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * transformedVertex;
}

