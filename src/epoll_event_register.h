#ifndef SRC_EPOLL_EVENT_REGISTER_H
#define SRC_EPOLL_EVENT_REGISTER_H

#include <map>
#include <sys/epoll.h>
#include <unistd.h>

#include "event_register.h"

/**
 * Epoll event register for file descriptor.
 */
class EpollEventRegister : public EventRegister {
 public:
    EpollEventRegister() : _cur_fd_num(0) {
        _epoll_fd = epoll_create(MAX_FDS_NUM);
        CHECK_EQ(_epoll_fd >= 0);
    }
    virtual ~EpollEventRegister() {
        close(_epoll_fd);
    }

    /**
     * Use level-trigger mode.
     */
    virtual bool register_fd(int fd, EventFlag flag);


    virtual bool unregister_fd(int fd, EventFlag flag);
    virtual bool is_registered(int fd, EventFlag flag);
    virtual void wait_events(std::vector<int> *readable_fds,
        std::vector<int> *writable_fds, std::map<int, int>* error_fds);

 private:
    int _epoll_fd;

    std::map<int, int> _fds_status;
    unsigned int _cur_fd_num;
    struct epoll_event _ready_events[MAX_FDS_NUM];
};

#endif  // SRC_EPOLL_EVENT_REGISTER_H
