#include <languageserver.h>
#include <httpd.h>
#include <lexer.h>
#include <string.h>

void language_server(int c, char **v) {
    serve_forever("12913");
}

void route() {
    ROUTE_START()

    POST("/highlight")
    {
        const char *output = lexer_highlight(payload);
        _puts("HTTP/1.1 200 OK\r\n\r\n");
        _puts(output);
    }
  
    ROUTE_END()
}
