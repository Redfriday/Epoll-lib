#include <iostream>

#include "check.h"
#include "epoll_event_register.h"
#include "method_thread.h"
#include "scoped_lock.h"

#include "event_manager.h"

EventManager *EventManager::instance = NULL;
static pthread_once_t is_initialized = PTHREAD_ONCE_INIT;

static void event_manager_init() {
    EventManager::instance = new EventManager();
}

EventManager * EventManager::Instance() {
	pthread_once(&is_initialized, event_manager_init);
	return instance;
}

EventManager::EventManager() : _pending_change(false) {
    _event_register = new EpollEventRegister();

    CHECK_EQ(pthread_mutex_init(&_m, NULL) == 0);
    CHECK_EQ(pthread_cond_init(&_change_done_c, NULL) == 0);
    CHECK_EQ((_th = method_thread(this, false, &EventManager::wait_events_loop)) != 0);
}

EventManager::~EventManager() {
}

bool EventManager::add_callback(int fd, EventFlag flag, EventCallback* callback) {
    ScopedLock ml(&_m);
    if (!_event_register->register_fd(fd, flag)) {
        return false;
    }

    CHECK_EQ(_callbacks.find(fd) == _callbacks.end() || _callbacks[fd] == callback);
    _callbacks[fd] = callback;

    return true;
}

void EventManager::block_remove_fd(int fd) {
    ScopedLock ml(&_m);
    (void)_event_register->unregister_fd(fd, EVENT_RW);
    _pending_change = true;
    CHECK_EQ(pthread_cond_wait(&_change_done_c, &_m) == 0);
    _callbacks.erase(fd);
}

void EventManager::del_callback(int fd, EventFlag flag) {
    ScopedLock ml(&_m);
    if (_event_register->unregister_fd(fd, flag)) {
        _callbacks.erase(fd);
    }
}

bool EventManager::has_callback(int fd, EventFlag flag, EventCallback* callback) {
    ScopedLock ml(&_m);
    if (_callbacks.find(fd) == _callbacks.end() || _callbacks[fd] != callback) {
        return false;
    }

    return _event_register->is_registered(fd, flag);
}

void EventManager::wait_events_loop() {
    std::vector<int> readable_fds;
    std::vector<int> writable_fds;
    std::map<int, int> error_fds;

    while (true) {
        {
            ScopedLock ml(&_m);
            if (_pending_change) {
                _pending_change = false;
                CHECK_EQ(pthread_cond_broadcast(&_change_done_c) == 0);
            }
        }
        readable_fds.clear();
        writable_fds.clear();
        error_fds.clear();
        _event_register->wait_events(&readable_fds, &writable_fds, &error_fds);
        if (readable_fds.empty() && writable_fds.empty() && error_fds.empty()) {
            continue;
        } 
        
        for (auto it = error_fds.begin(); it != error_fds.end(); it++) {
            int fd = it->first;
            int status = it->second;
            if (_callbacks.find(fd) != _callbacks.end()) {
                _callbacks[fd]->error_cb(fd, status);
            }
        }
        // We don't lock here because no add_callback() and del_callback
        // should modify _callbacks[fd] while the fd is not dead.
        for (unsigned int i = 0; i < readable_fds.size(); i++) {
            int fd = readable_fds[i];
            if (_callbacks.find(fd) != _callbacks.end()) {
                _callbacks[fd]->read_cb(fd);
            }
        }
        for (unsigned int i = 0; i < writable_fds.size(); i++) {
            int fd = writable_fds[i];
            if (_callbacks.find(fd) != _callbacks.end()) {
                _callbacks[fd]->write_cb(fd);
            }
        }
    }
}
