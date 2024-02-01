#pragma once
#include <cstdint>

class CWatchScheduler
{
public:
	CWatchScheduler();
	~CWatchScheduler();

	void StartScheduler();
	void StopScheduler();
	
	
	DWORD GetLCTime(int &wday);
	void SynchronizeTimeState();
	void SynchronizeTimeStateLite();
	void SynchronizeTimeState(int wday, DWORD lctime);
	void SetTimeState(DWORD tstate);
	const DWORD GetTimeState() const { return lState; };

private:
	pthread_t tid;
	bool isSchedulerRunning;
	DWORD nInterval, lCurDate, lState;
	CJonecConfig::JONEC_SCHEDULE schedule;

	inline BOOL IsDayEnd(DWORD*, int*) const;
	inline void UpdateCurrentDate();
	
	static void WatchTimerProc(void *p);
};

