#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

layout (std430, binding = 3) buffer ssbo {
	mat4 instanceMatrices[];
};

out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * instanceMatrices[gl_InstanceID] * vec4(aPos, 1.0f);
	TexCoords = aTexCoords;
};
