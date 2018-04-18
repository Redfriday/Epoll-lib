#ifndef SRC_EVENT_CALLBACK
#define SRC_EVENT_CALLBACK

/**
 * Abstract event callback class.
 */
class EventCallback {
 public:
    virtual void read_cb(int fd) = 0;
    virtual void write_cb(int fd) = 0;
    virtual void error_cb(int fd, int events) = 0;
    virtual ~EventCallback() {}
};

#endif  // SRC_EVENT_CALLBACK
