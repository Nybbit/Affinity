#version 330 core
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;

out vec2 Uv;
out vec4 Color;

uniform mat4 cameraMatrix;

void main()
{
	gl_Position = cameraMatrix * vec4(inPosition, 0.0, 1.0);
	Uv = vec2(inUv.x, 1.0 - inUv.y);
	Color = inColor;
}
