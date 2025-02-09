#include "constants.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"
#include <time.h>

char server_message[MAX_MSG_LEN];

int main(int argc, char *argv[]) {
    // Initialize signal handlers for watchdog monitoring.
    HANDLE_WATCHDOG_SIGNALS();

    // Variables for server communication pipes.
    int to_server_pipe, from_server_pipe;

    // Validate command-line arguments: Expecting exactly 2 pipes.
    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server_pipe);
        sscanf(argv[2], "%d", &from_server_pipe);
    } else {
        printf("Error: Invalid number of arguments in obstacles\n");
        getchar();  
        exit(1);
    }

    // Obstacle-related Variables
    float obstacle_pos_x, obstacle_pos_y;  // Coordinates for obstacles.

    // Server Communication Buffers
    char obstacle_data[MAX_MSG_LEN] = "O";  // Message identifier for obstacles.
    char formatted_message[MAX_MSG_LEN] = {0};  // Buffer for composing messages.

    // Random Number Generator Initialization
    // Seeds the generator with the current time (multiplied by 33 for variation),
    // ensuring different obstacle positions across program executions.
    srandom((unsigned int)time(NULL) * 33);

    // File Descriptor Sets for Monitoring Pipes
    fd_set read_fds, master_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&master_fds);
    FD_SET(from_server_pipe, &master_fds);

    // Timeout Settings for Select()
    struct timeval select_timeout;
    select_timeout.tv_sec = OBSTACLES_SPAWN_PERIOD;
    select_timeout.tv_usec = 0;

    while (1) {
        // Generate and format a new set of obstacle coordinates to send to the server.

        // Start message with obstacle count in protocol format: "[N]"
        sprintf(formatted_message, "[%d]", N_OBSTACLES);
        strcat(obstacle_data, formatted_message);

        for (int i = 0; i < N_OBSTACLES; i++) {
            if (i != 0) {
                strcat(obstacle_data, "|");  // Separate multiple obstacles with "|"
            }

            // Generate random obstacle coordinates within the simulation boundaries.
            obstacle_pos_x = random() % SIMULATION_WIDTH;
            obstacle_pos_y = random() % SIMULATION_HEIGHT;

            // Format as "x,y" with three decimal places.
            sprintf(formatted_message, "%.3f,%.3f", obstacle_pos_x, obstacle_pos_y);
            strcat(obstacle_data, formatted_message);
        }

        // Send the formatted message to the server.
        Write(to_server_pipe, obstacle_data, MAX_MSG_LEN);

        // Reset message buffer for the next iteration.
        sprintf(obstacle_data, "O");

        // Log successful obstacle generation.
        logging("INFO", "Obstacles process generated a new set of obstacles");

        // Reset the file descriptor set for the next select() call.
        read_fds = master_fds;

        int select_result;
        do {
            select_result = Select(from_server_pipe + 1, &read_fds, NULL, NULL, &select_timeout);
        } while (select_result == -1);  // Retry if select() is interrupted.

        // Reset timeout for the next iteration.
        select_timeout.tv_sec  = OBSTACLES_SPAWN_PERIOD;
        select_timeout.tv_usec = 0;

        // Check if there is data to read from the server.
        if (FD_ISSET(from_server_pipe, &read_fds)) {
            int read_ret = Read(from_server_pipe, server_message, MAX_MSG_LEN);

            if (read_ret == 0) {
                // If the pipe is closed, remove it from the set and log the event.
                Close(from_server_pipe);
                FD_CLR(from_server_pipe, &master_fds);
                logging("WARN", "Pipe to obstacles closed");
            }

            // If the received message is "STOP", terminate the process.
            if (!strcmp(server_message, "STOP")) {
                break;
            }
        }
    }

    // Cleanup before exiting
    Close(to_server_pipe);
    return 0;
}