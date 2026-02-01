#ifndef CONTROL_H
#define CONTROL_H

#include "handlers.h"

// Shared globals (declared as extern so they don't double-allocate)
extern handle_req_t handle_request_func;

// Function Prototypes
handle_req_t load_handlers();


#endif
