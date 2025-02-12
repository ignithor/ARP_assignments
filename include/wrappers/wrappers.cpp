#include "wrappers/wrappers.h"  
#include <cstdio>               
#include <cstdlib>             
#include <cstring>              
#include <cerrno>              
#include <csignal>            
#include <signal.h>             
#include <unistd.h>            
#include <sys/wait.h>          
#include <sys/types.h>         
#include <sys/stat.h>          
#include <fcntl.h>              
#include <sys/file.h>           
#include <stdexcept>         
#include <sstream>             
#include <iostream>             
#include <string>               
#include <thread>           
#include <chrono>              
#include <vector>               
#include "utility/utility.h"

int Wait(int *wstatus) {
    int ret = wait(wstatus);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing wait: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);

        logging("ERROR", msg.str().c_str()); // Ensure `logging` supports C++ strings

        std::cin.get(); // Equivalent to `getchar()` in C++
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Waitpid(pid_t pid, int *wstatus, int options) {
    int ret = waitpid(pid, wstatus, options);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing waitpid: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);

        logging("ERROR", msg.str().c_str()); // Ensure `logging` supports C++ strings

        std::cin.get(); // Equivalent to `getchar()` in C++
        exit(EXIT_FAILURE);
    }
    return ret;
}


int Execvp(const char *file, char **args) {
    int ret = execvp(file, args);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing execvp: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);

        logging("ERROR", msg.str().c_str()); // Ensure `logging` supports C++ strings

        std::cin.get(); // Equivalent to `getchar()` in C++
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Fork() {
    int ret = fork();
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing fork: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);

        logging("ERROR", msg.str().c_str()); // Ensure `logging` supports C++ strings

        std::cin.get(); // Equivalent to `getchar()` in C++
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Read(int fd, void *buf, size_t nbytes) {
    // Ignore SIGPIPE to prevent crashing if the pipe is broken
    struct sigaction ignore_pipesig;
    ignore_pipesig.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &ignore_pipesig, nullptr);

    // Perform the read operation
    int ret = read(fd, buf, nbytes);

    // Restore default behavior for SIGPIPE
    ignore_pipesig.sa_handler = SIG_DFL;
    sigaction(SIGPIPE, &ignore_pipesig, nullptr);

    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing read: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);

        logging("ERROR", msg.str().c_str()); // Ensure `logging` supports C++ strings

        // Sleep for 100 seconds before termination
        std::this_thread::sleep_for(std::chrono::seconds(100));
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Write(int fd, const void *buf, size_t nbytes) {
    // Ignore SIGPIPE to prevent process crashes
    struct sigaction ignore_pipesig;
    ignore_pipesig.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &ignore_pipesig, nullptr);

    // Perform write operation
    int ret = write(fd, buf, nbytes);

    // Restore default SIGPIPE behavior
    ignore_pipesig.sa_handler = SIG_DFL;
    sigaction(SIGPIPE, &ignore_pipesig, nullptr);

    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing write: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(100));
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    return select(nfds, readfds, writefds, exceptfds, timeout);
}

int Select_wmask(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    // Temporarily block SIGUSR1 to prevent interruption of the select syscall
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block_mask, nullptr);

    int ret = select(nfds, readfds, writefds, exceptfds, timeout);

    // Unblock SIGUSR1 after select execution
    sigprocmask(SIG_UNBLOCK, &block_mask, nullptr);

    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error executing select: " << strerror(errno)
            << ", pid: " << getpid()
            << ", file: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Open(const char *file, int oflag) {
    int ret = open(file, oflag);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing open: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(100));
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Pipe(int *pipedes) {
    int ret = pipe(pipedes);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing pipe: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(100));
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Close(int fd) {
    int ret = close(fd);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing close: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(100));
        exit(EXIT_FAILURE);
    }
    return ret;
}

int Flock(int fd, int operation) {
    int ret = flock(fd, operation);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error executing flock: " << strerror(errno)
            << ", pid: " << getpid()
            << ", file: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        exit(EXIT_FAILURE);
    }
    return ret;
}

FILE *Fopen(const char *pathname, const char *mode) {
    FILE *ret = fopen(pathname, mode);
    if (!ret) {
        std::ostringstream msg;
        msg << "Error on executing fopen: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(100));
        exit(EXIT_FAILURE);
    }
    return ret;
}

void Kill(int pid, int signal) {
    int ret = kill(pid, signal);
    if (ret < 0) {
        std::ostringstream msg;
        msg << "Error on executing kill: " << strerror(errno)
            << ", pid: " << getpid()
            << ", from: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        exit(EXIT_FAILURE);
    }
}


int Kill2(int pid, int signal) {
    return (kill(pid, signal) < 0) ? -1 : 0;
}


void Mkfifo(const char *fifo_path, int permit) {
    if (access(fifo_path, F_OK) < 0) {
        if (mkfifo(fifo_path, permit) < 0) {
            std::ostringstream msg;
            msg << "Error executing mkfifo: " << strerror(errno)
                << ", pid: " << getpid()
                << ", file: " << __FILE__
                << ", line: " << __LINE__
                << ", awaiting termination from WD";

            std::cerr << msg.str() << std::endl;
            std::fflush(stdout);
            logging("ERROR", msg.str().c_str());

            std::this_thread::sleep_for(std::chrono::seconds(2));
            exit(EXIT_FAILURE);
        }
    }
}

void Sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (sigaction(signum, act, oldact) < 0) {
        std::ostringstream msg;
        msg << "Error executing sigaction: " << strerror(errno)
            << ", pid: " << getpid()
            << ", file: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        exit(EXIT_FAILURE);
    }
}

void Sigprocmask(int type, const sigset_t *mask, sigset_t *oldset) {
    if (sigprocmask(type, mask, oldset) < 0) {
        std::ostringstream msg;
        msg << "Error executing sigprocmask: " << strerror(errno)
            << ", pid: " << getpid()
            << ", file: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        exit(EXIT_FAILURE);
    }
}

void Fclose(FILE *stream) {
    if (std::fclose(stream) == EOF) {
        std::ostringstream msg;
        msg << "Error executing fclose: " << strerror(errno)
            << ", pid: " << getpid()
            << ", file: " << __FILE__
            << ", line: " << __LINE__
            << ", awaiting termination from WD";

        std::cerr << msg.str() << std::endl;
        std::fflush(stdout);
        logging("ERROR", msg.str().c_str());

        exit(EXIT_FAILURE);
    }
}
