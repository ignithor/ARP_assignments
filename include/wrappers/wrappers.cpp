#include "wrappers/wrappers.hpp"  

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

int Write(int fd, void *buf, size_t nbytes) {
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