#version 430 core

in vec3 pos;
in vec3 normal;
flat in int instanceID;

out vec4 finalColor;

void main() {

	finalColor = vec4(normalize(abs(pos)), 1.0f);
}