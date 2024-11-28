#pragma once

#include <stdio.h>
#include <string.h>

struct Settings;

// Client request
extern char *method, // "GET" or "POST"
    *uri,            // "/index.html" things before '?'
    *qs,             // "a=1&b=2" things after  '?'
    *prot,           // "HTTP/1.1"
    *payload;        // for POST

extern int payload_size;

// Server control functions
void serve_forever(const char *PORT, struct Settings*);

char *request_header(const char *name);

typedef struct {
  char *name, *value;
} header_t;
static header_t reqhdr[17] = {{"\0", "\0"}};
header_t *request_headers(void);

// user shall implement this function

void route();

// Response
#define RESPONSE_PROTOCOL "HTTP/1.1"

#define HTTP_200 _puts("HTTP/1.1 200 OK\n\n")
#define HTTP_201 _puts("HTTP/1.1 201 Created\n\n")
#define HTTP_404 _puts("HTTP/1.1 404 Not found\n\n")
#define HTTP_500 _puts("HTTP/1.1 500 Internal Server Error\n\n")

// some interesting macro for `route()`
#define ROUTE_START() if (0) {
#define ROUTE(METHOD, URI)                                                     \
  }                                                                            \
  else if (_strcmp(URI, uri) == 0 && _strcmp(METHOD, method) == 0) {
#define GET(URI) ROUTE("GET", URI)
#define POST(URI) ROUTE("POST", URI)
#define ROUTE_END()                                                            \
  }                                                                            \
  else HTTP_500;
