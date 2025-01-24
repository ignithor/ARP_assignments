#include "constants.h"
#include "wrapFuncs/wrapFunc.h"
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utils/utils.h>

// WD pid
pid_t WD_pid;

char received[MAX_MSG_LEN];

int main(int argc, char *argv[]) {
    // Macro to handle the watchdog signals
    HANDLE_WATCHDOG_SIGNALS();

    // Specifying that argc and argv are unused variables
    int to_server_pipe, from_server_pipe;

    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server_pipe);
        sscanf(argv[2], "%d", &from_server_pipe);
    } else {
        printf("Wrong number of arguments in obstacles\n");
        getchar();
        exit(1);
    }

    // coordinates of obstacles
    float obstacle_x, obstacle_y;
    // string for the communication with server
    char to_send[MAX_MSG_LEN] = "O";
    // String to compose the message for the server
    char aux_to_send[MAX_MSG_LEN] = {0};

    // seeding the random nymber generator with the current time, so that it
    // starts with a different state every time the programs is executed
    srandom((unsigned int)time(NULL) * 33);

    fd_set reader, master;
    FD_ZERO(&reader);
    FD_ZERO(&master);
    FD_SET(from_server_pipe, &master);

    struct timeval select_timeout;
    select_timeout.tv_sec  = OBSTACLES_SPAWN_PERIOD;
    select_timeout.tv_usec = 0;
    while (1) {
        // spawn random coordinates in map field range and send it to the
        // server, so that they can be spawned in the map, and format as
        // specified in the protocol
        sprintf(aux_to_send, "[%d]", N_OBSTACLES);
        strcat(to_send, aux_to_send);
        for (int i = 0; i < N_OBSTACLES; i++) {
            if (i != 0) {
                strcat(to_send, "|");
            }
            // The obstacle has to stay inside the simulation window
            obstacle_x = random() % SIMULATION_WIDTH;
            obstacle_y = random() % SIMULATION_HEIGHT;
            sprintf(aux_to_send, "%.3f,%.3f", obstacle_x, obstacle_y);
            strcat(to_send, aux_to_send);
        }
        // Sending to the server
        Write(to_server_pipe, to_send, MAX_MSG_LEN);

        // Resetting to_send string
        sprintf(to_send, "O");

        // Logging the correct generation
        logging(LOG_INFO, "Obstacles process generated a new set of obstacles");

        // Resetting the fd_sets
        reader = master;
        int ret;
        do {
            ret = Select(from_server_pipe + 1, &reader, NULL, NULL,
                         &select_timeout);
        } while (ret == -1);
        // Resetting the timeout
        select_timeout.tv_sec  = OBSTACLES_SPAWN_PERIOD;
        select_timeout.tv_usec = 0;
        if (FD_ISSET(from_server_pipe, &reader)) {
            int read_ret = Read(from_server_pipe, received, MAX_MSG_LEN);
            if (read_ret == 0) {
                // If closed pipe close fd
                Close(from_server_pipe);
                FD_CLR(from_server_pipe, &master);
                logging(LOG_WARN, "Pipe to obstacles closed");
            }
            // If STOP received then stop everything
            if (!strcmp(received, "STOP")) {
                break;
            }
        }
    }

    // Cleaning up
    Close(to_server_pipe);
    return 0;
}
