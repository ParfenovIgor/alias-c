#include "../include/languageserver.h"
#include "../include/httpd.h"
#include "../include/lexer.h"
#include "../stdlib/include/string.h"

void LanguageServer(int c, char** v) {
    serve_forever("12913");
}

void route() {
    ROUTE_START()

    GET("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Hello! You are using %s", request_header("User-Agent"));
    }

    POST("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Wow, seems that you POSTed %d bytes. \r\n", payload_size);
        printf("Fetch the data using `payload` variable.");
    }

    POST("/highlight")
    {
        const char *output = Lexer_Highlight(payload);
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("%.*s", (int)strlen(output), output);
    }
  
    ROUTE_END()
}
