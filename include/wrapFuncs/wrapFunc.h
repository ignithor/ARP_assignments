#ifndef WRAPFUNC
#define WRAPFUNC
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/signal.h>

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
int Shm_open(const char *name, int flag, __mode_t mode);
int Ftruncate(int fd, __off_t length);
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, __off_t offset);
sem_t *Sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int Sem_init(sem_t *sem, int pshared, int value);
int Sem_wait(sem_t *sem);
int Sem_post(sem_t *sem);
int Sem_close(sem_t *sem);
int Sem_unlink(const char *name);
FILE *Fopen(const char *pathname, const char *mode);
int Flock(int fd, int operation);
void Kill(int pid, int signal);
int Kill2(int pid, int signal);
void Mkfifo(const char *fifo_path, int permit);
void Sigaction(int signum, const struct sigaction *act,
               struct sigaction *oldact);
void Sigprocmask(int type, const sigset_t *mask, sigset_t *oldset);
void Fclose(FILE *stream);
void Shm_unlink(const char *name);
void Munmap(void *addr, size_t len);
#endif // !WRAPFUNC
