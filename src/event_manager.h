#ifndef SRC_EVENT_MANAGER_H
#define SRC_EVENT_MANAGER_H

#include <map>
#include <pthread.h>

#include "event_callback.h"
#include "event_register.h"

class EventManager {
 public:
    EventManager();
    ~EventManager();

    static EventManager *Instance();

    bool add_callback(int fd, EventFlag flag, EventCallback *callback);
    void del_callback(int fd, EventFlag flag);
    bool has_callback(int fd, EventFlag flag, EventCallback *callback);

    /** 
     * Remove all callbacks related to fd
     */
    void block_remove_fd(int fd);

    void wait_events_loop();

    static EventManager *instance;

 private:
    pthread_mutex_t _m;
    pthread_cond_t _change_done_c;
    pthread_t _th;

    std::map<int, EventCallback*> _callbacks;
    EventRegister *_event_register;
    bool _pending_change;
};

#endif  // SRC_EVENT_MANAGER_H
