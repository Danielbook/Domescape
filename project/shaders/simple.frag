#version 330 core

uniform sampler2D Tex;
uniform sampler2D shadowMap;
uniform vec4 sunColor;
uniform float fAmbInt;

in vec3 lDir;
in vec2 UV;
in vec3 tnormals;
in vec4 ShadowCoord;

out vec4 color;

/*
float GetVisibility(sampler2D shadowMap, vec4 vShadowCoord)
{
    float visibility = 1.0;

    if(texture( shadowMap, vShadowCoord.xy ).z  <  vShadowCoord.z)
    {
		visibility = 0.0;
		float bias = 0.005;
		vShadowCoord /= vShadowCoord.w;

		for(int i = 0; i < 16; i++)
		{
			float depth = texture(shadowMap, vShadowCoord.st);

			if(vShadowCoord.z - bias < depth)
			{
				visibility += 1.0;
			}
		}
		visibility /= 16.0;
	}

    return visibility;
}
*/

void main()
{
// SHADOWMAP

float visibility = 1.0;
if ( texture( shadowMap, ShadowCoord.xy ).z  <  ShadowCoord.z)
{
    visibility = 0.5;
}

// float visibility = GetVisibility(shadowMap, ShadowCoord);

 //PHONG from TNM046

// vec3 L is the light direction
// vec3 V is the view direction - (0 ,0 ,1) in view space
// vec3 N is the normal
// vec3 R is the computed reflection direction
// float n is the " shininess " parameter
// vec3 ka is the ambient reflection color
// vec3 Ia is the ambient illumination color
// vec3 kd is the diffuse surface reflection color
// vec3 Id is the diffuse illumination color
// vec3 ks is the specular surface reflection color
// vec3 Is is the specular illumination color
// This assumes that N, L and V are normalized .

vec4 vTexColor = texture(Tex, UV.st);
vec3 V = vec3(0.0,0.0,1.0);
vec3 L = lDir;
vec3 N = tnormals;
float n = 10;
vec3 ka = vec3(fAmbInt-0.1,fAmbInt-0.1,fAmbInt-0.1);
vec3 Ia = vec3(fAmbInt,fAmbInt,fAmbInt);
vec3 kd = vec3(sunColor-0.2);
vec3 Id = vec3(sunColor-0.15);
vec3 ks = vec3(sunColor-0.1);
vec3 Is = vec3(sunColor);

vec3 R = 2.0* dot (N ,L) *L - L; // Could also have used the function reflect ()
float dotNL = max ( dot (N , L) , 0.0) ; // If negative , set to zero
float dotRV = max ( dot (R , V) , 0.0) ;
if ( dotNL == 0.0) dotRV = 0.0; // Do not show highlight on the dark side
vec3 shadedcolor = Ia * ka + visibility * Id * kd * dotNL + visibility * Is * ks * pow ( dotRV , n);
color =  vTexColor * vec4( shadedcolor , 1.0) ;

}
