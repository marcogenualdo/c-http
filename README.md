# Nginx style C HTTP Server

Fun exercise to write a simple C HTTP server with threads and handlers runtime reloading.

It was made to explore these UNIX/Linux concepts:

* Dynamic library loading
* Linux processes and tasks
* Unix domain sockets and IPC
* Signals handling
* Poll and non blocking I/O
* Syscalls and strace

### Features

* Serves static files in the `www` directory.
* Nginx style worker processes.
* Runtime reloads HTTP handler library via Unix sockets.
* Graceful shutdown of worker processes on SIGTERM / SIGINT.

### Limitations (I just don't care)

* Extremely basic HTTP 1.1. GET requests only, only parsing method and path.
* No request validation. It doesn't check method or path (except file permissions). A malformed request may even crash the process.

This was not an HTTP compliance exercise. I chose synthesis for future refrence over robustness.


# How it works

Split into data plane, and control plane.

* Control plane (`server.c`): main loop. Listens for reload events of signals.
* Data plane (`worker.c`): worker processes. Handle requests.
* Handler library (`handlers.c`, loaded at runtime): contains the (extremely basic) HTTP handlers.


# How to use

Most recipes are in the Makefile, to build just run `make`.

Start the server and watch syscalls:

```bash
strace -f -e trace=clone,network,read,write ./chttp

```

Run a request:

```bash
wget -S http://localhost:8080/ -qO - # 200 - OK
wget -S http://localhost:8080/notfound -qO - # 404 - Not Found

```

Hot reload:

```bash
echo "RELOAD" | socat - UNIX-CONNECT:/tmp/server.control
```
