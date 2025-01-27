#ifndef WRAPFUNC
#define WRAPFUNC
#include "constants.h"
#include "utils/utils.h"
#include <curses.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int Wait(int *wstatus);
int Waitpid(pid_t pid, int *wstatus, int options);
int Execvp(const char *file, char **args);
int Read(int fd, void *buf, size_t nbytes);
int Write(int fd, void *buf, size_t nbytes);
int Fork(void);
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout);
int Select_wmask(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                 struct timeval *timeout);
int Open(const char *file, int oflag);
int Pipe(int *pipedes);
int Close(int fd);
FILE *Fopen(const char *pathname, const char *mode);
int Flock(int fd, int operation);
void Kill(int pid, int signal);
int Kill2(int pid, int signal);
void Mkfifo(const char *fifo_path, int permit);
void Sigaction(int signum, const struct sigaction *act,
               struct sigaction *oldact);
void Sigprocmask(int type, const sigset_t *mask, sigset_t *oldset);
void Fclose(FILE *stream);
#endif // !WRAPFUNC
