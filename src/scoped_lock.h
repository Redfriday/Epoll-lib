#ifndef SRC_SCOPED_LOCK_H
#define SRC_SCOPED_LOCK_H

#include <pthread.h>

#include "check.h"

struct ScopedLock {
 public:
    ScopedLock(pthread_mutex_t *m): _m(m) {
        CHECK_EQ(pthread_mutex_lock(_m) == 0);
    }

    ~ScopedLock() {
        CHECK_EQ(pthread_mutex_unlock(_m) == 0);
    }

 private:
    pthread_mutex_t *_m;
};

#endif  // SRC_SCOPED_LOCK_H
