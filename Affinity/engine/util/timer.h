#pragma once

/*
 * This class may be a little confusing but it's primary use is to
 * be able to modify values independently from the framerate.
 * The timer assumes the desired update rate is 60Hz
 *
 * Values are doubles because the additional precision is necessary
 * for calculating at higher update rates (such as 6000Hz). After
 * calculation, (for example the getDelta() function) is should be
 * safe to cast to a float.
 *
 * [Example]
 * Without frame independence:
 * - moveForward() { m_position.z += 1.0f; }
 * - Movement depends on framerate, the more updates there are the faster the movement
 *
 * With frame independence:
 * - moveForward(float delta) { m_position.z += 1.0f * delta; }
 * - Movement no longer depends on framerate
 * - A computer that updates twice as much in the same amount of time will result in
 *   the same distance moved as a computer that doesn't.
 */
class Timer final
{
public:
	Timer();

	/**
	 * \brief Update the amount of times the timer has been updated in the last second
	 */
	void update();

	/**
	 * \brief Get how many times per second the timer is being updated
	 */
	int getUpdatesPerSecond() const;

	/**
	 * \brief Get the amount of frames passed since last update (assuming 60Hz)
	 * \return Amount of frames passed since last update
	 */
	double getDelta() const;
private:
	/**
	 * \brief Get the time in milliseconds since the application started
	 * \return Time in milliseconds
	 */
	static double getTime();

	/**
	 * \brief Calculate delta
	 * \return Frames (60Hz) passed since last update
	 */
	double calculateDelta();

	int m_updatesPerSecond = 0;
	double m_delta         = 0.0;

	double m_lastFrame            = 0;
	int m_currentUpdatesPerSecond = 0;
	double m_lastUpdatePerSecond  = 0.0;
};
