#version 330 core


uniform sampler2D shadowMap;
uniform float near;
uniform float far;

in vec2 UV;

out vec4 color;

layout(location = 0) out float fragmentdepth;


float LinearizeDepth()
{
  float z = texture(shadowMap, UV.st).x;
  return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{
//    float depth = texture(shadowMap, UV).x;
//    depth = 1.0 - (1.0 - depth) * 25.0;
//    color = vec4(depth);


//    vec4 texel = texture(shadowMap, UV.st);
//	if( UV.x < 0.5 )
//		color = texel;
//	else
//	{
//		float d = LinearizeDepth();
//		color = vec4(d, d, d, 1.0);
//		//color = vec4(1.0-d, sqrt(d), d, 1.0);
//	}

	fragmentdepth = gl_FragCoord.z;
}

