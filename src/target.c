#include "constants.h"
#include "utils/utils.h"
#include "wrapFuncs/wrapFunc.h"
#include <time.h>

int main(int argc, char *argv[]) {
    // Macro to handle the watchdog signals
    HANDLE_WATCHDOG_SIGNALS();

    // Specifying that argc and argv are unused variables
    int to_server_pipe, from_server_pipe;

    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server_pipe);
        sscanf(argv[2], "%d", &from_server_pipe);
    } else {
        printf("Wrong number of arguments in target\n");
        getchar();
        exit(1);
    }

    // coordinates of target
    float target_x, target_y;
    // string for the communication with server
    char to_send[MAX_MSG_LEN] = "T";
    // String to compose the message for the server
    char aux_to_send[MAX_MSG_LEN] = {0};

    // seeding the random nymber generator with the current time, so that it
    // starts with a different state every time the programs is executed
    srandom((unsigned int)time(NULL));

    char received[MAX_MSG_LEN];

    while (1) {
        // Resetting to_send string
        sprintf(to_send, "T");
        // spawn random coordinates in map field range and send it to the
        // server, so that they can be spawned in the map
        sprintf(aux_to_send, "[%d]", N_TARGETS);
        strcat(to_send, aux_to_send);
        for (int i = 0; i < N_TARGETS; i++) {
            if (i != 0) {
                strcat(to_send, "|");
            }
            // Targets need to be inside the simulation window
            target_x = random() % SIMULATION_WIDTH;
            target_y = random() % SIMULATION_HEIGHT;
            sprintf(aux_to_send, "%.3f,%.3f", target_x, target_y);
            strcat(to_send, aux_to_send);
        }

        // Send new targets to server
        Write(to_server_pipe, to_send, MAX_MSG_LEN);

        // Here Read is used instead of Select because it has to be blocking
        // untill the server sends a new GE
        Read(from_server_pipe, received, MAX_MSG_LEN);
        if (!strcmp(received, "GE")) {
            logging("INFO", "Received GE");
        } else if (!strcmp(received, "STOP")) {
            // Otherwise if it's STOP close everything
            break;
        }
        // Log if new targets have been produced
        logging("INFO", "Target process generated a new set of targets");
    }

    // Cleaning up
    Close(to_server_pipe);
    return 0;
}
