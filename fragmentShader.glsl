#version 330 core	     // Minimal GL version support expected from the GPU

struct Material {
// ...
	sampler2D albedoTex; // texture unit, relate to glActivateTexture(GL_TEXTURE0 + i)
};
uniform Material material;
in vec2 fTexCoord;

uniform vec3 worldPos;
uniform vec3 camPos;
uniform vec3 surfaceColor;
uniform vec3 lightPos;
uniform int isLight;
uniform int isSky;
in vec3 fPosition;
in vec3 fNormal;
out vec4 color;	  // Shader output: the color response attached to this fragment

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float ka = 0.1;
const float kd = 1.0;
const float ks = 0.8;
const float shininess = 2.0;

void main() {
	if (isLight != 0) {
		vec3 texColor = texture(material.albedoTex, fTexCoord).rgb;
		color = vec4(0.8 * texColor, 1);
	} 
	else if (isSky != 0) {
		vec3 texColor = texture(material.albedoTex, fTexCoord).rgb;
		color = vec4(0.8 * texColor, 1);
	}
	else {

		vec3 texColor = texture(material.albedoTex, fTexCoord).rgb; // sample the texture color
		vec3 n = normalize(fNormal);
		vec3 l = normalize(lightPos - worldPos);
		vec3 v = normalize(camPos - fPosition);
		vec3 r = reflect(-l, n);

		vec3 ambient = ka * lightColor;
		vec3 diffuse = kd * max(dot(n, l), 0.0) * texColor * lightColor;
		vec3 specular;
		if (dot(n, l) > 0) {
    		specular = ks * pow(max(dot(v, r), 0.0), shininess) * texColor * lightColor;
		} else {
    		specular = vec3(0.0, 0.0, 0.0);
		}
	
		color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.
	}
}
