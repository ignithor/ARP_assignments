#include "constants.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"
#include <time.h>

int main(int argc, char *argv[]) {
    // Handle watchdog signals
    HANDLE_WATCHDOG_SIGNALS();

    // Validate input arguments and extract pipe file descriptors
    int to_server_pipe, from_server_pipe;
    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server_pipe);
        sscanf(argv[2], "%d", &from_server_pipe);
    } else {
        printf("Error: Incorrect number of arguments in target process\n");
        getchar();
        exit(1);
    }

    // Variables for target coordinates
    float target_pos_x, target_pos_y;
    
    // Buffers for communication with the server
    char msg_to_send[MAX_MSG_LEN] = "T";  // Message to send targets
    char aux_to_send[MAX_MSG_LEN] = {0}; // Temporary buffer for formatting
    char server_response[MAX_MSG_LEN]; // Buffer for received messages

    // Seed the random number generator with current time for unique results
    srandom((unsigned int)time(NULL));

    while (1) {
        // Reset target message buffer
        sprintf(msg_to_send, "T");

        // Format message with number of targets
        sprintf(aux_to_send, "[%d]", N_TARGETS);
        strcat(msg_to_send, aux_to_send);

        // Generate random target positions
        for (int i = 0; i < N_TARGETS; i++) {
            if (i != 0) strcat(msg_to_send, "|"); // Separate targets with "|"

            // Ensure targets remain within simulation boundaries
            target_pos_x = random() % SIMULATION_WIDTH;
            target_pos_y = random() % SIMULATION_HEIGHT;

            // Append formatted target coordinates to message
            sprintf(aux_to_send, "%.3f,%.3f", target_pos_x, target_pos_y);
            strcat(msg_to_send, aux_to_send);
        }

        // Send newly generated targets to the server
        Write(to_server_pipe, msg_to_send, MAX_MSG_LEN);

        // Wait for the server’s response (blocking read)
        Read(from_server_pipe, server_response, MAX_MSG_LEN);

        // Process received server message
        if (!strcmp(server_response, "GE")) {
            logging("INFO", "Received GE (Generate Event) signal");
        } else if (!strcmp(server_response, "STOP")) {
            // If STOP signal is received, terminate the process
            break;
        }

        // Log successful target generation
        logging("INFO", "New target positions generated and sent to the server");
    }

    // Cleaning up
    Close(to_server_pipe);

    return 0;
}
