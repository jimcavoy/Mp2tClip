#pragma once
#include <cstdint>

class PCRClock
{
public:
	PCRClock(void);
	~PCRClock(void);
	PCRClock(const PCRClock& other);
	PCRClock& operator=(const PCRClock& rhs);

public:
	void setTime(uint8_t* time);
	double timeInSeconds() const;
	uint64_t time() const;
	uint64_t baseTime() const;
	uint16_t extTime() const;

private:
	uint8_t _time[6]{};
};
