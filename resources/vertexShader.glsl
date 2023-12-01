/*
	vertexShader.glsl
	author: Telo PHILIPPE
*/

#version 460 core


layout(location=0) in vec2 vPosition;

out vec2 screenPos;

void main() {
	gl_Position = vec4(vPosition, 0.0, 1.0);
	screenPos = vPosition;
}
