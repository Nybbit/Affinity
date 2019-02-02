#include "timer.h"
#include <GLFW/glfw3.h>

Timer::Timer()
{
	m_lastUpdatePerSecond = static_cast<double>(getTime());
	m_delta               = calculateDelta();
}

void Timer::update()
{
	if (getTime() - m_lastUpdatePerSecond > 1000)
	{
		m_updatesPerSecond        = m_currentUpdatesPerSecond;
		m_currentUpdatesPerSecond = 0;
		m_lastUpdatePerSecond += 1000;
	}
	m_currentUpdatesPerSecond++;

	m_delta = calculateDelta();
}

int Timer::getUpdatesPerSecond() const
{
	return m_updatesPerSecond;
}

double Timer::getDelta() const
{
	return m_delta;
}

double Timer::getTime()
{
	return glfwGetTime() * 1000;
}

double Timer::calculateDelta()
{
	const auto time  = getTime();
	const auto delta = (time - m_lastFrame); // time in milliseconds since last update
	m_lastFrame      = time;

	return delta / (1000.0f / 60.0f); // how many frames (60hz) have passed since the last update
}
