#pragma once
struct Component
{
	Component() = default;
	virtual ~Component() = default;
	Component(const Component& other) = default;
	Component(Component&& other) noexcept = default;
	Component& operator=(const Component& other) = default;
	Component& operator=(Component&& other) noexcept = default;
};
