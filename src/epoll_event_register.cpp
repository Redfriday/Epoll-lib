#include <iostream>
#include <map>
#include <sys/epoll.h>

#include "check.h"
#include "epoll_event_register.h"

bool EpollEventRegister::register_fd(int fd, EventFlag flag) {
	struct epoll_event ev;
    int op = EPOLL_CTL_ADD;
    if (_fds_status.find(fd) != _fds_status.end()) {
        op = EPOLL_CTL_MOD;
    } else {
        if (_cur_fd_num >= MAX_FDS_NUM) {
            // TODO Add log: fd number reach limit.
            return false;
        }
        _cur_fd_num++;
    }

    int& status = _fds_status[fd];
    status |= (int)flag;

	ev.data.fd = fd;

	if (status & EVENT_RO) {
		ev.events |= EPOLLIN;
	}
	if (status & EVENT_WO) {
		ev.events |= EPOLLOUT;
	}

	CHECK_EQ(epoll_ctl(_epoll_fd, op, fd, &ev) == 0);

    return true;
}

bool EpollEventRegister::unregister_fd(int fd, EventFlag flag) {
    int& status = _fds_status[fd];
    status &= ~(int)flag;

	struct epoll_event ev;
	int op = status ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

	ev.data.fd = fd;

	if (status & EVENT_RO) {
		ev.events |= EPOLLIN;
	}
	if (status & EVENT_WO) {
		ev.events |= EPOLLOUT;
	}

	CHECK_EQ(epoll_ctl(_epoll_fd, op, fd, &ev) == 0);

    if (op == EPOLL_CTL_DEL) {
        _fds_status.erase(fd);
        return true;
    }

    return false;
}

bool EpollEventRegister::is_registered(int fd, EventFlag flag) {
    std::map<int, int>::iterator iter = _fds_status.find(fd);
    return iter != _fds_status.end() && iter->second == flag;
}

void EpollEventRegister::wait_events(std::vector<int>* readable_fds,
           std::vector<int>* writable_fds, std::map<int, int>* error_fds) {
	int n_fds = epoll_wait(_epoll_fd, _ready_events, MAX_FDS_NUM, -1);
	for (int i = 0; i < n_fds; i++) {
        int fd = _ready_events[i].data.fd;
        int events = (int)_ready_events[i].events;
        // If client close() or shutdown() function, we will receive EPOLLHUP.
        if (events & EPOLLHUP || events & EPOLLERR || events & EPOLLRDHUP) {
            (*error_fds)[fd] = (int)EVENT_ERROR; 
            continue;
        }
		if (events & EPOLLIN) {
			readable_fds->push_back(fd);
		}
		if (events & EPOLLOUT) {
			writable_fds->push_back(fd);
		}
	}
}
