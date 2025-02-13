#ifndef UTILITY_H
#define UTILITY_H

#include "constants.h"
#include "droneDataStructs.h"
#include "wrappers/wrappers.hpp"
#include <csignal>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <fstream>

void logging(const std::string &type, const std::string &message);
int max_of_many(int count, ...);
void signal_handler(int signo, siginfo_t *info, void *context);

// Macro to handle the watchdog signals for each process
#define HANDLE_WATCHDOG_SIGNALS()                                              \
    {                                                                          \
        struct sigaction sa;                                                   \
        std::memset(&sa, 0, sizeof(sa));                                       \
        sa.sa_sigaction = signal_handler;                                      \
        sigemptyset(&sa.sa_mask);                                              \
        sa.sa_flags = SA_SIGINFO | SA_RESTART;                                 \
        sigaction(SIGUSR1, &sa, nullptr);                                      \
    }

#endif // UTILITY_H
