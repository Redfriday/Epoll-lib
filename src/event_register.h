#ifndef SRC_EVENT_REGISTER_H
#define SRC_EVENT_REGISTER_H

#include <vector>
#include <map>

// Max registered fd number.
#define MAX_FDS_NUM 1024

/**
 * Event flag.
 */
typedef enum {
	EVENT_NONE = 0x00,
	EVENT_RO = 0x01,  // Read only.
	EVENT_WO = 0x10,  // Write only.
	EVENT_RW = 0x11,  // Read and write.
    EVENT_ERROR = 0x100,
	EVENT_MASK = ~0x111,
} EventFlag;

/** 
 * Abstract event register for file descriptor.
 */
class EventRegister {
 public:
    virtual bool register_fd(int fd, EventFlag flag) = 0;
    virtual bool unregister_fd(int fd, EventFlag flag) = 0;
    virtual bool is_registered(int fd, EventFlag flag) = 0;
    virtual void wait_events(std::vector<int>* readable_fds,
        std::vector<int>* writable_fds, std::map<int, int>* error_fds) = 0;
    virtual ~EventRegister() {}
};

#endif  // SRC_EVENT_REGISTER_H
