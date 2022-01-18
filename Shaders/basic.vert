#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 pos;
out vec3 normal;
flat out int instanceID;

uniform bool isInstanced = true;
uniform mat4 model = mat4(1.0f);
uniform mat4 view;
uniform mat4 projection;

struct InstanceData {
	mat4 model;
	vec4 color;
};

layout (std430, binding = 0) readonly buffer Instance {
	InstanceData instanceData[];
};

void main() {
	vec4 pos4 = vec4(aPos, 1.0f);
	if (isInstanced) {
		pos4 = instanceData[gl_InstanceID].model*pos4;
	} else {
		pos4 = model*pos4;
	}
	pos = pos4.xyz;
	normal = aNormal; //no inverse transpose because I'm just rendering spheres and axis aligned boxes
	instanceID = gl_InstanceID;
	gl_Position = projection*view*pos4;
}