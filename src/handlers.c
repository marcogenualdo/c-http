#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "responses.h"

#define ASSETS_PATH "/usr/share/chttp-server/www"

const char* get_www_path() {
    const char* env_path = getenv("CHTTP_WWW_PATH");
    if (env_path) return env_path;

    if (access("./www/index.html", F_OK) == 0) {
        return "./www";
    }

    return ASSETS_PATH;
}

char* handle_request(char* request_path) {
    // Don't allow ".." to escape the directory
    if (strstr(request_path, "..")) return strdup(HTTP_ERR_403);

    // Build path (if path is "/", serve index.html)
    char full_path[1024];
    const char* base_path = get_www_path();
    snprintf(full_path, sizeof(full_path), "%s%s", base_path, request_path);
    if (strcmp(request_path, "/") == 0) strcat(full_path, "index.html");

    FILE *f = fopen(full_path, "r");
    if (f == NULL) {
        return strdup(HTTP_ERR_404);
    }

    // Read file
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *response = malloc(fsize + 100);
    snprintf(response, fsize + 100, "%s %ld\n\n", HTTP_OK, fsize);
    fread(response + strlen(response), 1, fsize, f);

    fclose(f);
    return response;
}
