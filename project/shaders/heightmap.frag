#version 330 core

uniform sampler2D hTex;
uniform sampler2D nTex;
uniform float lAmb;

in vec2 UV;
in float vScale;
in vec3 lDir;
in mat3 NM;

out vec4 color;

// Computes the diffues shading by using the normal for
// the fragment and direction from fragment to the light
vec4 calcShading( vec3 N, vec3 L )
{
	//Ambient contribution
	vec4 Iamb = vec4(lAmb, lAmb, lAmb, 1.0);
	Iamb = clamp(Iamb, 0.0, 1.0);

	//Diffuse contribution
	vec4 light_diffuse = vec4(lAmb+0.3, lAmb+0.3, lAmb+0.3, 1.0);
	vec4 Idiff = light_diffuse * max(dot(N,L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);

	//Specular contribution
	vec3 V = vec3(0.0, 0.0, 1.0);
	vec4 light_specular = vec4(1.0, 1.0, 1.0, 1.0);
	const float specExp = 32.0f;

	//vec3 R = normalize(reflect(-L,N));
	vec3 R = normalize(2.0* dot (N ,L) *L - L);
	vec4 Ispec = light_specular	* pow(max(dot(R,V),0.0), specExp);
    Ispec = clamp(Ispec, 0.0, 1.0);

	return Iamb + Idiff + Ispec;
}

void main()
{

	vec3 pixelVals = texture( nTex, UV).rgb;
	vec3 normal;
	normal.x = (pixelVals.r * 2.0 - 1.0);
	normal.y = (pixelVals.b * 2.0 - 1.0)/vScale;
	normal.z = (pixelVals.g * 2.0 - 1.0);
	if(vScale < 0)
		normal = -normal;

	// Set fragment color
	// This will result in a non-linear color temperature scale based on height value
	float hVal = texture( hTex, UV).x;
	float Pi = 3.14159265358979323846264;
	color.rgb = vec3(0.3,sin(Pi*hVal),0.0);

	// multiply color with shading
	color.rgb *= calcShading( normalize(-NM * normal), lDir ).rgb;
	color.a = 1.0;
}
