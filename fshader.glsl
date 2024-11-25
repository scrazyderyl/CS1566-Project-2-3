#version 120

varying vec2 texCoord;
varying vec4 N, V, L;
varying float distance;

uniform sampler2D texture;
uniform int use_ambient, use_diffuse, use_specular, use_flashlight, lighting_enabled;
uniform float attenuation_constant, attenuation_linear, attenuation_quadratic;
uniform vec4 player_position;

vec4 ambient, diffuse, specular;

void main()
{
	if(lighting_enabled == 1) {
		vec4 the_color = texture2D(texture, texCoord);
		vec4 NN = normalize(N);
		vec4 LL = normalize(L);
		vec4 VV = normalize(V);
		vec4 H = normalize(LL + VV);
		if(use_ambient == 0 && use_diffuse == 0 && use_specular == 0 && use_flashlight == 1) {
			if (dot(LL, NN) < 0 && dot(LL, NN) > 1) {
				gl_FragColor = vec4(0, 0, 0, 1);

			}
			else {
				gl_FragColor = dot(LL, NN) * the_color * 5;
			}
		}
		else {
			if(use_ambient == 1) 
				ambient = 0.3 * the_color;
			else
				ambient = vec4(0, 0, 0, 0);
			if(use_diffuse == 1) 
				diffuse = max(dot(LL, NN), 0.0) * 0.8 * the_color;
			else
				diffuse = vec4(0, 0, 0, 1);
			if(use_specular == 1)
				specular = pow(max(dot(NN, H), 0.0), 50) * vec4(1.0, 1.0, 1.0, 1.0);
			else
				specular = vec4(0, 0, 0, 1);
			float attenuation = 1 / (attenuation_constant + (attenuation_linear * distance) + (attenuation_quadratic * distance * distance));
			gl_FragColor = ambient + (diffuse + specular);
		}
	}
	else
		gl_FragColor = texture2D(texture, texCoord);
}
