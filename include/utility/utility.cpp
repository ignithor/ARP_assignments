#include "utility/utility.hpp"

// Function to write log messages in the logfile
void logging(const std::string &type, const std::string &message) {
    std::ofstream logfile(LOGFILE_PATH, std::ios::app);
    if (logfile) {
        logfile << "[" << type << "] - " << message << std::endl;
    }
}

// Max function for several values
int max_of_many(int count, ...) {
    va_list args;
    va_start(args, count);
    int max_val = va_arg(args, int);
    for (int i = 1; i < count; i++) {
        int value = va_arg(args, int);
        if (value > max_val) {
            max_val = value;
        }
    }
    va_end(args);
    return max_val;
}

void signal_handler(int signo, siginfo_t *info, void *context) {
    (void)(context);
    if (signo == SIGUSR1) {
        pid_t WD_pid = info->si_pid;
        Kill(WD_pid, SIGUSR2);
    }
}
