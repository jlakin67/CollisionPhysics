#version 430 core

in vec3 pos;
in vec3 normal;
flat in int instanceID;

uniform bool isInstanced = true;
uniform vec4 color = vec4(vec3(207.0/255.0), 1.0);
uniform vec3 lightPos = vec3(0.0, 5.0, 0.0);
uniform vec4 skyColor = vec4(101.0 / 255.0, 226.0 / 255.0, 235.0 / 255.0, 1.0);
uniform vec4 groundColor = vec4(29.0/255.0, 54.0/255.0, 56.0/255.0, 1.0);

struct InstanceData {
	mat4 model;
	vec4 color;
};
layout (std430, binding = 0) readonly buffer Instance {
	InstanceData instanceData[];
};

out vec4 finalColor;

void main() {
	vec4 inputColor;
	if (isInstanced) inputColor = instanceData[instanceID].color;
	else inputColor = color;
	vec3 lightVec = normalize(lightPos - pos);
	float costheta = dot(normal, lightVec);
	float a = 0.5 * costheta + 0.5;
	finalColor = inputColor*mix(groundColor, skyColor, a);
}