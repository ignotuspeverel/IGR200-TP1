#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
//layout(location=2) in vec3 vColor;
uniform mat4 viewMat, projMat, modelMat; 
out vec3 fNormal;
out vec3 fPosition;

void main() {
        gl_Position = projMat * viewMat * modelMat * vec4(vPosition, 1.0); // mandatory to rasterize properly
        mat4 normalMatrix = transpose(inverse(mat4(modelMat)));
        fNormal = normalize(mat3(normalMatrix) * vNormal);
        //fNormal = vNormal;
        fPosition = (modelMat * vec4(vPosition, 1.0)).xyz;
        //fPosition = vPosition;
}
