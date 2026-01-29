#ifndef RESPONSES_H
#define RESPONSES_H

// Standard success
#define HTTP_OK "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"

// Error codes
#define HTTP_ERR_400 "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_ERR_403 "HTTP/1.1 403 Forbidden\n\nNo."
#define HTTP_ERR_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_ERR_414 "HTTP/1.1 414 URI Too Long\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_ERR_431 "HTTP/1.1 431 Request Header Fields Too Large\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#define HTTP_ERR_500 "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"

#endif
