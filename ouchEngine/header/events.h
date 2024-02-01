#ifndef __EVENTS_H__
#define __EVENTS_H__

#ifdef USE_KQUEUE_HANDLER
#include <sys/event.h>
#endif
#ifdef USE_EPOLL_HANDLER
#include <sys/epoll.h>
#endif

#define EVENT_READ 0x01
#define EVENT_WRITE 0x02

typedef enum
{
	EVENT_EPOLL,	// Linux
	EVENT_KQUEUE,	// BSD
	EVENT_DEVPOLL,	// Solaris
	EVENT_POLL,	// POSIX
	EVENT_SELECT	// Generic (Windows)
} fdevent_handler_t;

struct _fdevent
{
	int *basemem;
	//	Interfaces
	int (*add)(struct _fdevent*, int, int);
	int (*poll)(struct _fdevent*, int);
	int (*get_current_fd)(struct _fdevent*, int);
	void (*growup)(struct _fdevent*);
	int (*revent)(struct _fdevent*, int);
	#ifdef USE_KQUEUE_HANDLER
	int kq_fd;
	struct kevent *events;
	#endif
	#ifdef USE_EPOLL_HANDLER
	int epoll_fd;
	struct epoll_event *events;
	#endif
	fdevent_handler_t handler;
};

int events_init(struct _fdevent *ev, int *basemem);
int events_add(struct _fdevent *ev, int fd, int bidadd);
int events_poll(struct _fdevent *ev, int timeout_ms);
int events_get_current_fd(struct _fdevent *ev, int i);
void events_growup(struct _fdevent *ev);
int events_revent(struct _fdevent *ev, int i);

int event_kqueue_init(struct _fdevent *ev);
int event_epoll_init(struct _fdevent *ev);

#endif
