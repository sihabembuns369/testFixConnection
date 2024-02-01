#include "header/WatchScheduler.h"

CWatchScheduler::CWatchScheduler() : tid(0), isSchedulerRunning(false)
{
}

CWatchScheduler::~CWatchScheduler()
{
	StopScheduler();
}

void CWatchScheduler::StartScheduler()
{
	if (isSchedulerRunning)
		return;
	// pthread_create(&tid, NULL, (void *(*)(void *))WatchTimerProc, (void *)this);
}

void CWatchScheduler::StopScheduler()
{
	if (!isSchedulerRunning)
		return;
	isSchedulerRunning = false;
	// pthread_join(tid, NULL);
	tid = 0;
}

void CWatchScheduler::WatchTimerProc(void *p)
{
	CWatchScheduler *ws = (CWatchScheduler *)p;
	ws->isSchedulerRunning = true;
	while (ws->isSchedulerRunning)
	{
		// usleep(1000);
	}
}
