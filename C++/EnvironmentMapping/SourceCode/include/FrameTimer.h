#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#include <Windows.h>

class FrameTimer
{
private:
	double StartTime, EndTime, FrameTime;

public:
	FrameTimer();

	static double GetTimeHelper(void);

	void StartTimer(void);
	void EndTimer(void);
	double GetFrameTime(void) const;
};

#endif