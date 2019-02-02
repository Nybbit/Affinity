#pragma once
namespace config
{
	constexpr auto ROTATION_SPEED = 0.01f;

	constexpr auto MOVE_FORWARD = GLFW_KEY_W;
	constexpr auto TURN_LEFT = GLFW_KEY_A;
	constexpr auto TURN_RIGHT = GLFW_KEY_D;

	constexpr auto ZOOM_OUT = GLFW_KEY_DOWN;
	constexpr auto ZOOM_IN = GLFW_KEY_UP;

	static const auto BOAT_SIZE = glm::vec2(113, 66);
	static const auto CANNONBALL_SIZE = glm::vec2(10);
}
