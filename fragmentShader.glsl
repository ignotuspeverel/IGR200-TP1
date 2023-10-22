#version 330 core	     // Minimal GL version support expected from the GPU

uniform vec3 worldPos;
uniform vec3 camPos;
uniform vec3 surfaceColor;
uniform vec3 lightPos;
uniform int isLight;
in vec3 fPosition;
in vec3 fNormal;
out vec4 color;	  // Shader output: the color response attached to this fragment

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float ka = 0.3;
const float kd = 0.5;
const float ks = 0.5;
const float shininess = 16.0;

void main() {
	if (isLight != 0) {
		color = vec4(surfaceColor, 1.0);
	} else {
		//vec3 n = normalize(fNormal);
		//color = vec4((n + 1.0) * 0.5, 1.0); // normalize n values to [0, 1] range
		vec3 n = normalize(fNormal);
		//vec3 l = normalize(lightPos - fPosition);
		vec3 l = normalize(lightPos - worldPos);
		vec3 v = normalize(camPos - fPosition);
		vec3 r = reflect(-l, n);

		vec3 ambient = ka * lightColor;
		vec3 diffuse = kd * max(dot(n, l), 0.0) * surfaceColor * lightColor;
		vec3 specular;
		if (dot(n, l) > 0) {
    		specular = ks * pow(max(dot(v, r), 0.0), shininess) * surfaceColor * lightColor;
		} else {
    		specular = vec3(0.0, 0.0, 0.0);
		}
	
		color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.
	}
}
