#version 330 core

uniform sampler2D Tex;
uniform sampler2D glow;
uniform sampler2D colorSky;

uniform vec3 lightDir; //Calculated light direction

in vec3 position;
in vec2 UV;

out vec4 color;

void main()
{
    vec3 V = normalize(position);
    vec3 L = normalize(lightDir);

    // Compute the proximity of this fragment to the sun.
    float vl = dot(V, L);

    // Look up the sky color and glow colors.
    vec4 Kc = texture2D(colorSky, vec2((L.y + 1.0) / 2.0, V.y));
    vec4 Kg = texture2D(glow,  vec2((L.y + 1.0) / 2.0, vl));


    vec4 sTexColor = texture(Tex, UV.st);

    // Combine the color and glow giving the pixel value.
    //color = sTexColor * vec4(Kc.rgb + Kg.rgb * Kg.a / 2.0, Kc.a);
    color = sTexColor * vec4(Kc.rgb + Kg.rgb * Kg.a / 2.0, Kc.a);
}
