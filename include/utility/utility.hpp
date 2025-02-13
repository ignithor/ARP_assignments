#ifndef UTILITY_H
#define UTILITY_H

#include "constants.h"
#include "droneDataStructs.h"
#include "wrappers/wrappers.hpp"
#include <cjson/cJSON.h>
#include <csignal>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <vector>

float get_param(const std::string &process, const std::string &param);
void logging(const std::string &type, const std::string &message);
int max_of_many(int count, ...);
void tokenization(std::vector<pos> &arr_to_fill, const std::string &to_tokenize,
                  int &objects_num);
void remove_target(int index, std::vector<pos> &objects_arr, int objects_num);
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
