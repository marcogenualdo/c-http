#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "control.h"

void* lib_handle = NULL;

handle_req_t load_handlers() {
    handle_req_t handle_request_func = NULL;

    if (lib_handle) dlclose(lib_handle);

    lib_handle = dlopen("libchttp-handlers.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Failed to load: %s\n", dlerror());
        exit(1);
    }
    handle_request_func = dlsym(lib_handle, "handle_request");

    fprintf(stderr, "Plugin loaded/reloaded successfully.\n");
    return handle_request_func;
}
