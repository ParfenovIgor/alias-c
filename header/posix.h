#ifndef POSIX_H_INCLUDED
#define POSIX_H_INCLUDED

int posix_read(int fd, char *buffer, int count);
int posix_write(int fd, const char *buffer, int count);
int posix_open(const char *filename, int flags, int mode);
int posix_close(int fd);
int posix_fork();
int posix_execve(const char *filename, const char *const *argv, const char *const *envp);
void posix_exit(int error_code);
int posix_wait4(int pid, int *stat_addr, int options, void *rusage);

#endif // POSIX_H_INCLUDED
