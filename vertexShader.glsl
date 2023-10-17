#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
//layout(location=2) in vec3 vColor;

uniform mat4 viewMat, projMat; 
out vec3 fNormal;
out vec3 fPosition;

void main() {
        gl_Position = projMat * viewMat * vec4(vPosition, 1.0); // mandatory to rasterize properly
        fNormal = vNormal;
        fPosition = vPosition;
}
