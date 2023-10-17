#version 330 core	     // Minimal GL version support expected from the GPU

uniform vec3 camPos;
in vec3 fPosition;
in vec3 fNormal;
out vec4 color;	  // Shader output: the color response attached to this fragment

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 surfaceColor = vec3(0.0, 0.3, 0.3);
const float ka = 0.3;
const float kd = 0.5;
const float ks = 1;
const float shininess = 32.0;

void main() {
	//color = vec4(normalize(fNormal),1.0);
	vec3 n = normalize(fNormal);
	vec3 l = normalize(vec3(1.0, 1.0, 1.0));
	vec3 v = normalize(camPos - fPosition);
	vec3 r = reflect(-l, n);

	vec3 ambient = ka * lightColor;
	vec3 diffuse = kd * max(dot(n, l), 0.0) * surfaceColor * lightColor;
	vec3 specular = ks * pow(max(dot(v, r), 0.0), shininess) * surfaceColor * lightColor;
	color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.

}
