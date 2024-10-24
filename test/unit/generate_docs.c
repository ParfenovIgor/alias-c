#include <stdio.h>
#include <posix.h>
#include <stdbool.h>
#include <string.h>

const char *sections[1024];
const char *subsections[1024][1024];

void parse(int section_id, char *filename, int fd) {
    int file = posix_open(filename, 0, 0);
    int subsection_id = 1;

    bool global_header = true;
    bool local_header = true;
    bool code = false;

    while (true) {
        char buffer[4096];
        int len = 0;
        bool bad = true;
        while (posix_read(file, buffer + len, 1)) {
            if (buffer[len] == '\n') {
                bad = false;
                break;
            }
            if (buffer[len] == '<') {
                _strcpy(buffer + len, "&lt");
                len += 2;
            }
            if (buffer[len] == '>') {
                _strcpy(buffer + len, "&gt");
                len += 2;
            }
            len++;
        }
        if (bad) break;
        buffer[len] = '\0';
        char *line = buffer;
        
        if (!_strncmp(line, "//*", 3) && _strlen(line) >= 4) {
            if (code) {
                _fputs(fd, "</code></pre>\n");
                code = false;
            }
            line += 4;
            if (global_header) {
                _fputsi(fd, "<h2 id=\"", section_id, "\">");
                _fputs2(fd, line, "</h2>\n");
                sections[section_id] = _strdup(line);
                global_header = false;
            }
            else if (local_header) {
                _fputsi(fd, "<h3 id=\"", section_id, ".");
                _fputi(fd, subsection_id);
                _fputs3(fd, "\">", line, "</h3>\n");
                subsections[section_id][subsection_id] = _strdup(line);
                subsection_id++;
                local_header = false;
            }
            else {
                _fputs3(fd, "<p>", line, "</p>\n");
            }
        }
        else {
            if (!code && _strlen(line) && !global_header) {
                _fputs(fd, "<pre><code>");
                code = true;
                local_header = true;
            }
            if (code) {
                _fputs2(fd, line, "\n");
            }
        }
    }
    if (code) {
        _fputs(fd, "</code></pre>\n");
        code = false;
    }
    posix_close(file);
}

int main(int argc, char **argv) {
    int fd_contents[2];
    posix_pipe(fd_contents);
    for (int i = 1; i < argc; i++) {
        parse(i, argv[i], fd_contents[1]);
    }
    posix_close(fd_contents[1]);
    char *str = read_file_descriptor(fd_contents[0]);
    posix_close(fd_contents[0]);

    int out = STDOUT;
    
    _fputs(out, "<body>\n");
    _fputs(out, "<header><h1>Alias Language Reference</h1></header>\n");
    _fputs(out, "<div id=\"navigation\"><nav>\n");
    _fputs(out, "<h2>Table of Contents</h2>\n");
    _fputs(out, "<ul>");
    for (int i = 1; i < 1024; i++) {
        if (!sections[i]) break;
        _fputsi(out, "<li><a href=\"#", i, "\">");
        _fputs2(out, sections[i], "</a>\n");
        _fputs(out, "<ul>\n");
        for (int j = 1; j < 1024; j++) {
            if (!subsections[i][j]) break;
            _fputsi(out, "<li><a href=\"#", i, ".");
            _fputi(out, j);
            _fputs3(out, "\">", subsections[i][j], "</a></li>\n");
        }
        _fputs(out, "</ul></li>\n");
    }
    _fputs(out, "</ul></nav></div>\n");

    _fputs(out, "<div id=\"contents-wrapper\"><main id=\"contents\">\n");
    _fputs(out, str);
    _fputs(out, "</main></div></body></html>\n");

    return 0;
}