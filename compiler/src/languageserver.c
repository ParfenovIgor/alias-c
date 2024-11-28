#include <languageserver.h>
#include <posix.h>
#include <httpd.h>
#include <lexer.h>
#include <string.h>

const char *check_errors(const char *payload, struct Settings *settings) {
    int argc = 0;
    char *argv[1024];
    int n = _strlen(payload);
    for (int i = 0; i < n; i++) {
        if (payload[i] != ' ') {
            int j = i;
            while (j + 1 < n && payload[j + 1] != ' ') j++;
            int len = j - i + 1;
            char *arg = (char*)_malloc((len + 1) * sizeof(char));
            _strncpy(arg, payload + i, len);
            arg[len] = '\0';
            argv[argc] = arg;
            argc++;
            i = j;
        }
    }
    argv[argc] = NULL;

    int fd[2];
    posix_pipe(fd);
    int pid = posix_fork();
    if (pid == 0) {
        posix_dup2(fd[1], STDERR);
        posix_close(STDOUT);
        posix_execve(settings->calias_directory, (const char *const*)argv, 0);
    }
    posix_wait4(pid, 0, 0, 0);
    posix_close(fd[1]);
    const char *out = read_file_descriptor(fd[0]);

    n = _strlen(out);
    if (n == 0) {
        return "{\"res\":\"ok\"}";
    }

    int values[4];
    int pos = 0;
    const char *filename;
    for (int i = 0; i < 2; i++) {
        int l = pos;
        while (pos + 1 < n && out[pos + 1] != '\n') pos++;
        if (i == 1) {
            filename = _strndup(out + l, pos - l + 1);
        }
        pos += 2;
    }
    for (int i = 0; i < 4; i++) {
        while (!_isdigit(out[pos])) pos++;
        int x = (int)(out[pos] - '0');
        while (pos + 1 < n && _isdigit(out[pos + 1])) {
            x = x * 10 + (int)(out[pos + 1] - '0');
            pos++;
        }
        values[i] = x - 1;
        pos++;
    }
    int error_pos = pos;

    pos = 0;
    char *buffer = (char*)_malloc(1024 * sizeof(char));
    pos += _sputs(buffer + pos, "{\"res\":\"fail\"");
    pos += _sputs(buffer + pos, ",\"file\":\"");
    pos += _sputs(buffer + pos, filename);
    pos += _sputs(buffer + pos, "\"");
    pos += _sputs(buffer + pos, ",\"lb\":");
    pos += _sputi(buffer + pos, values[0]);
    pos += _sputs(buffer + pos, ",\"pb\":");
    pos += _sputi(buffer + pos, values[1]);
    pos += _sputs(buffer + pos, ",\"le\":");
    pos += _sputi(buffer + pos, values[2]);
    pos += _sputs(buffer + pos, ",\"pe\":");
    pos += _sputi(buffer + pos, values[3]);
    pos += _sputs(buffer + pos, ",\"err\":\"");
    pos += _sputs(buffer + pos, out + error_pos);
    pos += _sputs(buffer + pos, "\"}");
    buffer[pos] = '\0';

    return buffer;
}

void language_server(struct Settings *settings) {
    serve_forever("12913", settings);
}

void route(struct Settings *settings) {
    ROUTE_START()

    POST("/highlight")
    {
        const char *output = lexer_highlight(payload);
        _puts("HTTP/1.1 200 OK\r\n\r\n");
        _puts(output);
    }

    POST("/checkerrors")
    {
        const char *output = check_errors(payload, settings);
        _puts("HTTP/1.1 200 OK\r\n\r\n");
        _puts(output);
    }
  
    ROUTE_END()
}
