#version 120

varying vec2 texCoord;
varying vec4 N, V, L;

uniform sampler2D texture;

vec4 ambient, diffuse, specular;

void main()
{
	vec4 the_color = texture2D(texture, texCoord);
	vec4 NN = normalize(N);
	vec4 LL = normalize(L);
	vec4 VV = normalize(V);
	ambient = 0.3 * the_color;
	diffuse = max(dot(LL, NN), 0.0) * the_color;
	gl_FragColor = ambient + diffuse;
}
