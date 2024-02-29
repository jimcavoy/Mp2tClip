#include "PCRClock.h"
#include <memory.h>

const double RESOLUTION = 27000000.0; // 27MHz

PCRClock::PCRClock(void)
{
}

PCRClock::~PCRClock(void)
{
}

PCRClock::PCRClock(const PCRClock& other)
{
	memcpy(_time, other._time, 6);
}

PCRClock& PCRClock::operator=(const PCRClock& rhs)
{
	if (this != &rhs)
	{
		memcpy(_time, rhs._time, 6);
	}
	return *this;
}

void PCRClock::setTime(uint8_t* time)
{
	memcpy(_time, time, 6);
}

double PCRClock::timeInSeconds() const
{
	return (double)(time() / RESOLUTION);
}

uint64_t PCRClock::time() const
{
	uint64_t pcr = baseTime() * 300 + extTime();
	return pcr;
}

uint64_t PCRClock::baseTime() const
{
	uint64_t pcr_base = ((uint64_t)_time[0] << (33 - 8)) |
		((uint64_t)_time[1] << (33 - 16)) |
		((uint64_t)_time[2] << (33 - 24)) |
		((uint64_t)_time[3] << (33 - 32));

	return pcr_base;
}

uint16_t PCRClock::extTime() const
{
	uint16_t pcr_ext = _time[4] & 0x01 << 9;
	pcr_ext = pcr_ext | _time[5];

	return pcr_ext;
}
