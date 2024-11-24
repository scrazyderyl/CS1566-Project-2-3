#version 120

varying vec2 texCoord;
varying vec4 N, V, L;

uniform sampler2D texture;
uniform int use_ambient;
uniform int use_diffuse;
uniform int use_specular;
uniform int lighting_enabled;

vec4 ambient, diffuse, specular;

void main()
{
	if(lighting_enabled == 1) {
		vec4 the_color = texture2D(texture, texCoord);
		vec4 NN = normalize(N);
		vec4 LL = normalize(L);
		vec4 VV = normalize(V);
		vec4 H = normalize(LL + VV);
		if(use_ambient == 1) 
			ambient = 0.3 * the_color;
		else
			ambient = vec4(0, 0, 0, 0);
		if(use_diffuse == 1) 
			diffuse = max(dot(LL, NN), 0.0) * the_color;
		else
			diffuse = vec4(0, 0, 0, 0);
		if(use_specular == 1)
			specular = pow(max(dot(NN, H), 0.0), 200) * vec4(1.0, 1.0, 1.0, 1.0);
		else
			specular = vec4(0, 0, 0, 0);
		gl_FragColor = ambient + diffuse + specular;
	}
	else
		gl_FragColor = texture2D(texture, texCoord);
}
