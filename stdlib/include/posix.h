#pragma once

#define	PROT_NONE	0x00
#define	PROT_READ	0x01
#define	PROT_WRITE	0x02
#define	PROT_EXEC	0x04

#define	MAP_SHARED	0x01
#define	MAP_PRIVATE	0x02
#define	MAP_ANON	0x20
#define	MAP_ANONYMOUS	MAP_ANON

#define STDIN       0x00
#define STDOUT      0x01
#define STDERR      0x02

int posix_read(int fd, char *buffer, int count);
int posix_write(int fd, const char *buffer, int count);
int posix_open(const char *filename, int flags, int mode);
int posix_close(int fd);
void *posix_mmap(void *start, int length, int prot, int flags, int fd, int offset);
int posix_munmap(void *start, int length);
int posix_fork();
int posix_execve(const char *filename, const char *const *argv, const char *const *envp);
void posix_exit(int error_code);
int posix_wait4(int pid, int *stat_addr, int options, void *rusage);
int posix_unlink(const char *pathname);
