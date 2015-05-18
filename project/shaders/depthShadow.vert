#version 330 core

layout(location = 0) in vec3 vertexPositions;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec2 inCoords;

uniform mat4 depthMVP;

out vec2 UV;

void main()
{
    gl_Position =  depthMVP * vec4(vertexPositions,1);

    UV = inCoords;

}
