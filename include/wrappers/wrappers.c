#include "wrappers/wrappers.h"

int Wait(int *wstatus) {
    int ret = wait(wstatus);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing wait: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Waitpid(pid_t pid, int *wstatus, int options) {
    int ret = waitpid(pid, wstatus, options);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing waitpid: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Execvp(const char *file, char **args) {
    int ret = execvp(file, args);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing execvp: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Fork(void) {
    int ret = fork();
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing fork: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Read(int fd, void *buf, size_t nbytes) {
    // The following lines ignore the SIGPIPE signal in
    // onder to net cause the process who receives it to crash
    // but allows the handling of the signal in this function
    struct sigaction ignore_pipesig;
    ignore_pipesig.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &ignore_pipesig, NULL);
    // Here the actual read is performed and if any SIGPIPE is detected a
    // perror will be issued
    int ret                   = read(fd, buf, nbytes);
    ignore_pipesig.sa_handler = SIG_DFL;
    sigaction(SIGPIPE, &ignore_pipesig, NULL);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing read: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        sleep(100);
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Write(int fd, void *buf, size_t nbytes) {
    // The following lines ignore the SIGPIPE signal in
    // onder to net cause the process who receives it to crash
    // but allows the handling of the signal in this function
    struct sigaction ignore_pipesig;
    ignore_pipesig.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &ignore_pipesig, NULL);
    // Here the actual write is performed and if any SIGPIPE is detected a
    // perror will be issued
    int ret                   = write(fd, buf, nbytes);
    ignore_pipesig.sa_handler = SIG_DFL;
    sigaction(SIGPIPE, &ignore_pipesig, NULL);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing write: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        sleep(100);
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout) {
    int ret = select(nfds, readfds, writefds, exceptfds, timeout);
    return ret;
}

int Select_wmask(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                 struct timeval *timeout) {
    // Temporarily blocking the SIGUSR1 signal to correctly perform the
    // select() syscall without being interrupted. Since the time taken from
    // the select to execute is significantly lower than the WD period for
    // sending signals, this mask should not affect the WD behaviour

    // Block SIGUSR1
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    Sigprocmask(SIG_BLOCK, &block_mask, NULL);

    int ret = select(nfds, readfds, writefds, exceptfds, timeout);
    // unblock SIGUSR1
    Sigprocmask(SIG_UNBLOCK, &block_mask, NULL);

    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing select: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Open(const char *file, int oflag) {
    int ret = open(file, oflag);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing open: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Pipe(int *pipedes) {
    int ret = pipe(pipedes);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing pipe: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Close(int fd) {
    int ret = close(fd);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing close: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Flock(int fd, int operation) {
    int ret = flock(fd, operation);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing flock: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

FILE *Fopen(const char *pathname, const char *mode) {
    FILE *ret = fopen(pathname, mode);
    if (ret == NULL) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing fopen: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
    return ret;
}

void Kill(int pid, int signal) {
    int ret = kill(pid, signal);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing kill: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        exit(EXIT_FAILURE);
    }
}

int Kill2(int pid, int signal) {
    int ret = kill(pid, signal);
    if (ret < 0)
        return -1;
    return 0;
}

void Mkfifo(const char *fifo_path, int permit) {
    if (access(fifo_path, F_OK) < 0) {
        if (mkfifo(fifo_path, permit) < 0) {
            char msg[MAX_STR_LEN];
            sprintf(msg,
                    "Error on executing mkfifo: %s, pid: %d, from: %s, line: "
                    "%d, awaiting "
                    "termination from WD",
                    strerror(errno), getpid(), __FILE__, __LINE__);
            printf("%s\n", msg);
            fflush(stdout);
            logging("ERROR", msg);
            getchar();
            exit(EXIT_FAILURE);
        }
    }
}

void Sigaction(int signum, const struct sigaction *act,
               struct sigaction *oldact) {
    int ret = sigaction(signum, act, oldact);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing sigaction: %s, pid: %d, from: %s, line: "
                "%d, awaiting "
                "termination from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
}

void Sigprocmask(int type, const sigset_t *mask, sigset_t *oldset) {
    int ret = sigprocmask(type, mask, oldset);
    if (ret < 0) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing sigprocmask: %s, pid: %d, from: %s, line: "
                "%d, awaiting "
                "termination from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        getchar();
        exit(EXIT_FAILURE);
    }
}

void Fclose(FILE *stream) {
    int ret = fclose(stream);
    if (ret == EOF) {
        char msg[MAX_STR_LEN];
        sprintf(msg,
                "Error on executing fclose: %s, pid: %d, from: %s, line: %d, "
                "awaiting "
                "termination "
                "from WD",
                strerror(errno), getpid(), __FILE__, __LINE__);
        printf("%s\n", msg);
        fflush(stdout);
        logging("ERROR", msg);
        exit(EXIT_FAILURE);
    }
}