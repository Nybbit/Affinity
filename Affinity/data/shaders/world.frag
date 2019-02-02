#version 330 core

out vec4 FragColor;

uniform sampler2D tex;
uniform vec4 uv;

uniform vec2 cameraPos;
uniform vec2 cameraScale;
uniform vec2 screenResolution;

const float TILE_SIZE = 180;

void main()
{
	vec2 offset = mod(((gl_FragCoord.xy - screenResolution / 2) / cameraScale + cameraPos) / TILE_SIZE, 1);

	FragColor = texture(tex, vec2(uv.x, 1.0 - uv.y) + offset * vec2(uv.z, -uv.w));
}
