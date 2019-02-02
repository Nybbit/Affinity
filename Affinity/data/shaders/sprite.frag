#version 330 core
in vec2 Uv;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D tex;

void main()
{
	FragColor = texture(tex, Uv) * Color;
}
