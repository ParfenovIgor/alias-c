#include "../include/languageserver.h"
#include "../include/httpd.h"
#include "../include/lexer.h"
#include "../stdlib/include/string.h"

#define BUFFER_SIZE 10000

void LanguageServer(int c, char** v) {
    serve_forever("12913");
}

const char *tokenColor(enum TokenType type) {
    if (type == TokenIdentifier) return "yellow";
    if (type == TokenInteger) return "green";
    if (type == TokenString) return "red";
    if (type == TokenFunc) return "blue";
    return "black";
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

    GET("/highlight")
    {
        if (!request_header("filename")) {
            printf("HTTP/1.1 400 Bad Request\r\n\r\n");
            return;
        }
        
        const char *filename = request_header("filename");
        char *buffer = ReadFile(filename);
        if (!buffer) {
            printf("HTTP/1.1 400 Bad Request\r\n\r\n");
            return;
        }
        struct TokenStream *token_stream = Lexer_Process(buffer, filename);

        char output[BUFFER_SIZE];
        int pos = 0;

        output[pos] = '['; pos++;

        for (int i = 0; token_stream->stream[i].type != TokenEof; i++) {
            int cnt = sprintf(output + pos,
                "{\"lb\":%d,\"pb\":%d,\"le\":%d,\"pe\":%d,\"cl\":\"%s\"},",
                token_stream->stream[i].line_begin,
                token_stream->stream[i].position_begin,
                token_stream->stream[i].line_end,
                token_stream->stream[i].position_end,
                tokenColor(token_stream->stream[i].type));
            pos += cnt;
        }
        output[pos] = ']'; pos++;

        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("%.*s", pos, output);
    }
    
    POST("/highlight")
    {
        char *buffer = (char*)_malloc(payload_size + 1);
        strncpy(buffer, payload, payload_size);
        buffer[payload_size] = '\0';
        struct TokenStream *token_stream = Lexer_Process(buffer, "");
        _free(buffer);

        char output[BUFFER_SIZE];
        int pos = 0;

        output[pos] = '['; pos++;

        for (int i = 0; token_stream->stream[i].type != TokenEof; i++) {
            int cnt = sprintf(output + pos,
                "{\"lb\":%d,\"pb\":%d,\"le\":%d,\"pe\":%d,\"cl\":\"%s\"},",
                token_stream->stream[i].line_begin,
                token_stream->stream[i].position_begin,
                token_stream->stream[i].line_end,
                token_stream->stream[i].position_end,
                tokenColor(token_stream->stream[i].type));
            pos += cnt;
        }
        output[pos] = ']'; pos++;

        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("%.*s", pos, output);
    }
  
    ROUTE_END()
}
