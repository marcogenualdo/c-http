#ifndef CONTROL_H
#define CONTROL_H

#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "handlers.h"

// Shared globals (declared as extern so they don't double-allocate)
extern pthread_rwlock_t reload_lock;
extern volatile atomic_bool reload_needed;
extern handle_req_t handle_request_func;

// Function Prototypes
void load_plugin(void);
void* control_thread_func(void* arg);


#endif
