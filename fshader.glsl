#version 120

varying vec2 texCoord;
varying vec4 N, V, L;

uniform sampler2D texture;
uniform int use_ambient;
uniform int use_diffuse;
uniform int use_specular;

vec4 ambient, diffuse, specular;

void main()
{
	vec4 the_color = texture2D(texture, texCoord);
	vec4 NN = normalize(N);
	vec4 LL = normalize(L);
	vec4 VV = normalize(V);
	if(use_ambient == 1) 
		ambient = 0.3 * the_color;
	if(use_diffuse == 1) 
		diffuse = max(dot(LL, NN), 0.0) * the_color;
	if(use_ambient == 0 && use_diffuse == 0 && use_specular == 0)
		gl_FragColor = the_color;
	else
		gl_FragColor = ambient + diffuse;
}
