# Nginx style C HTTP Server

Fun exercise to write a simple C HTTP server with threads and handlers hot reloading.

It was made to explore these UNIX/Linux concepts:

* Dynamic vs static library linking
* Runtime ABI loading
* Linux threads and tasks
* Unix domain sockets and IPC
* Syscalls and strace


### Features

* Serves static files in the `www` directory.
* Multi threaded. One thread per connection.
* Hot reloads HTTP handler library via Unix sockets.


# How it works

Split into data plane, and control plane.

* Data plane (`server.c`): main loop. Listens for TCP connections, and dispatches them to threads.
* Control plane (`control.c`, statically linked into main binary): spanws a thread to listen for reload requests. If a reload request is received, it alerts the data plane to reload the handler functions before accepting new requests.
* Handler library (`handler.c`, linked at runtime): contains the (extremely basic) HTTP handlers.


# How to use

Most recipes are in the makefile.

Start the server and watch syscalls:

```bash
strace -f -e trace=clone,network,read,write ./server

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
