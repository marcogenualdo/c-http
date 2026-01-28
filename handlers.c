#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "responses.h"

// This is the "API" our server expects
char* handle_request(char* request_path) {
    // Basic security: don't allow ".." to escape the directory!
    if (strstr(request_path, "..")) return strdup(HTTP_ERR_403);

    // If path is "/", serve index.html
    char path[256] = "www";
    strcat(path, request_path);
    if (strcmp(request_path, "/") == 0) strcat(path, "index.html");

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return strdup(HTTP_ERR_404);
    }

    // Read file (Simplified for brevity)
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *response = malloc(fsize + 100);
    sprintf(response, "%s %ld\n\n", HTTP_OK, fsize);
    fread(response + strlen(response), 1, fsize, f);

    fclose(f);
    return response;
}
