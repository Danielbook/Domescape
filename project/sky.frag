
uniform sampler2D glow;
uniform sampler2D color;

in vec3 vertex;

out vec4 color;

void main()
{
    vec3 V = normalize(vertex);
    vec3 L = normalize(gl_LightSource[0].position.xyz);

    // Compute the proximity of this fragment to the sun.
    float vl = dot(V, L);

    // Look up the sky color and glow colors.
    vec4 Kc = texture2D(color, vec2((L.y + 1.0) / 2.0, V.y));
    vec4 Kg = texture2D(glow,  vec2((L.y + 1.0) / 2.0, vl));

    // Combine the color and glow giving the pixel value.
    color = vec4(Kc.rgb + Kg.rgb * Kg.a / 2.0, Kc.a);
}
