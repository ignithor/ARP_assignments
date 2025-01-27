#pragma once
#include "constants.h"
#include "dataStructs.h"
#include "wrapFuncs/wrapFunc.h"
#include <cjson/cJSON.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

float get_param(const char *process, const char *param);
void logging(char *type, char *message);
int max_of_many(int count, ...);
void tokenization(struct pos *arr_to_fill, char *to_tokenize, int *objects_num);
void remove_target(int index, struct pos *objects_arr, int objects_num);
void signal_handler(int signo, siginfo_t *info, void *context);

// Macro to handle the watchdog signals for each process
#define HANDLE_WATCHDOG_SIGNALS()                                              \
    {                                                                          \
        struct sigaction sa;                                                   \
        memset(&sa, 0, sizeof(sa));                                            \
        sa.sa_sigaction = signal_handler;                                      \
        sigemptyset(&sa.sa_mask);                                              \
        sa.sa_flags = SA_SIGINFO | SA_RESTART;                                 \
        sigaction(SIGUSR1, &sa, NULL);                                         \
    }
