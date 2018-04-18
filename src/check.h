#ifndef SRC_CHECK_EQ_H
#define SRC_CHECK_EQ_H

#include <stdlib.h>
#include <assert.h>

#ifdef NDEBUG
#define CHECK_EQ(expr) do { if (!(expr)) abort(); } while (0)
#else
#define CHECK_EQ(expr) assert(expr)
#endif

#endif  // SRC_CHECK_EQ_H
