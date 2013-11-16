/********************************************************************
**FrameTimer.cpp
**
**-Implements the FrameTimer class, used to determine framerate to
** track performance and allow movement speed based on time
********************************************************************/

#include "SystemModules.h"

//-----FrameTimer Class Functions-----//
FrameTimer::FrameTimer()
{
	StartTime = 0.0;
	EndTime = 0.0;
	FrameTime = 0.0;
}

double FrameTimer::GetTimeHelper(void)
{
	static bool FirstTime = true;
	static LARGE_INTEGER PF;
	LARGE_INTEGER PC;

	double Ret = 0.0;
	
	// if this is the 1st call to the function, 
	// query the performance counter frequency
	if(FirstTime)
	{
		QueryPerformanceFrequency(&PF);
		FirstTime = false;
	}
	
	if(PF.QuadPart)
	{
		QueryPerformanceCounter(&PC);

		double R0, R1;

		R0	= double(PC.QuadPart / PF.QuadPart);
		R1	= double(PC.QuadPart % PF.QuadPart) / double(PF.QuadPart);
		Ret	= R0 + R1;
	}
	else
	{
		Ret = timeGetTime() * 0.001;
	}

	return Ret;
}

void FrameTimer::StartTimer(void)
{
	StartTime = this->GetTimeHelper();
}

void FrameTimer::EndTimer(void)
{
	EndTime = this->GetTimeHelper();
	FrameTime = EndTime - StartTime;
}

double FrameTimer::GetFrameTime(void) const
{
	return FrameTime;
}
//-----FrameTimer Class Functions-----//