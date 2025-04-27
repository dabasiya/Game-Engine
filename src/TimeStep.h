#pragma once

struct TimeStep {
	float m_time;

	TimeStep(float time)
		: m_time(time)
	{
	}

	operator float() const { return m_time; }

	float getsecond() const { return m_time; }
	float getmillisecond() const { return m_time * 1000.0f; }
};
