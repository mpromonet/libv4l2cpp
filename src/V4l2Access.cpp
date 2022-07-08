/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Access.cpp
**
** V4L2 wrapper
**
** -------------------------------------------------------------------------*/

#include "V4l2Access.h"

#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "logger.h"

V4l2Access::V4l2Access(V4l2Device* device) : m_device(device) {
}

V4l2Access::~V4l2Access() { 
	delete m_device; 
}

bool V4l2Access::poll(timeval* timeout, V4l2Access::Mode mode) {
	int fd = m_device->getFd();
	if (fd == -1)
	{
		LOG(ERROR) << "File descriptor(-1) error";
		return false;
	}

	int epollfd = epoll_create(1);
	if (epollfd < 0)
	{
		char buf[1024];
		char* str = strerror_r(errno, buf, sizeof(buf));
		LOG(ERROR) << "Can't create epoll queue: " << str;
		return false;
	}

	struct epoll_event evin;
	memset(&evin, 0, sizeof(evin));

	if (mode & V4l2Access::READ) evin.events |= EPOLLIN;
	if (mode & V4l2Access::WRITE) evin.events |= EPOLLOUT;
	if (mode & V4l2Access::ERROR) evin.events |= EPOLLERR;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &evin) < 0)
	{
		char buf[1024];
		char* str = strerror_r(errno, buf, sizeof(buf));
		::close(epollfd);
		LOG(ERROR) << "Can't insert socket to epoll queue: " << str;
		return false;
	}

	long remaining_ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
	int rc;
	do
	{
		struct epoll_event evout;
		memset(&evout, 0, sizeof(evout));

		struct timeval start;
		gettimeofday(&start, NULL);
		long start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
		rc            = epoll_wait(epollfd, &evout, 1, remaining_ms);
		if (rc < 0 && errno == EINTR)
		{
			struct timeval end;
			gettimeofday(&end, NULL);
			long end_ms    = end.tv_sec * 1000 + end.tv_usec / 1000;
			long waited_ms = end_ms - start_ms;
			if (waited_ms < remaining_ms)
			{
				remaining_ms -= waited_ms;
			}
			else
			{
				remaining_ms = 0;
			}
		}
	} while (rc < 0 && errno == EINTR);

	::close(epollfd);
	return rc > 0;
}