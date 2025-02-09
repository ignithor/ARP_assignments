#include "constants.h"
#include "droneDataStructs.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"

int main(int argc, char *argv[]) {
    // Initialize Watchdog Signal Handling
    HANDLE_WATCHDOG_SIGNALS();

    // Pipes for inter-process communication (IPC)
    int from_drone_pipe, to_drone_pipe;
    int from_input_pipe, to_input_pipe;
    int from_map_pipe, to_map_pipe;
    int from_target_pipe, to_target_pipe;
    int from_obstacles_pipe, to_obstacle_pipe;

    // Verify Argument Count
    if (argc == 11) {
        // Extract pipe file descriptors from command-line arguments
        sscanf(argv[1], "%d", &from_drone_pipe);
        sscanf(argv[2], "%d", &to_drone_pipe);
        sscanf(argv[3], "%d", &from_input_pipe);
        sscanf(argv[4], "%d", &to_input_pipe);
        sscanf(argv[5], "%d", &from_map_pipe);
        sscanf(argv[6], "%d", &to_map_pipe);
        sscanf(argv[7], "%d", &from_target_pipe);
        sscanf(argv[8], "%d", &to_target_pipe);
        sscanf(argv[9], "%d", &from_obstacles_pipe);
        sscanf(argv[10], "%d", &to_obstacle_pipe);
    } else {
        // Handle incorrect argument count
        printf("Server: Error - Incorrect number of arguments.\n");
        getchar();
        exit(1);
    }

    // Structures to store drone information
    struct pos drone_current_pos = {0};
    struct velocity drone_current_velocity = {0};

    // Buffers for logging and message exchange
    char received_msg[MAX_MSG_LEN];
    char msg_to_send[MAX_MSG_LEN];

    // File descriptor sets for monitoring multiple input sources
    fd_set reader;
    fd_set master;

    // Initialize file descriptor sets
    FD_ZERO(&reader);
    FD_ZERO(&master);
    FD_SET(from_drone_pipe, &master);
    FD_SET(from_input_pipe, &master);
    FD_SET(from_map_pipe, &master);
    FD_SET(from_obstacles_pipe, &master);
    FD_SET(from_target_pipe, &master);

    // Determine the highest file descriptor value for select()
    int max_fd_value = max_of_many(5, from_drone_pipe, from_input_pipe, from_map_pipe,
                            from_obstacles_pipe, from_target_pipe);  


    bool stop_requested = false;

    while (1) {
        // Reset the file descriptor set for select()
        reader = master;
        Select_wmask(max_fd_value + 1, &reader, NULL, NULL, NULL);

        // Process each active file descriptor
        for (int i = 0; i <= max_fd_value; i++) {
            if (FD_ISSET(i, &reader)) {
                int read_bytes = Read(i, received_msg, MAX_MSG_LEN);
                
                // Handle closed pipes
                if (read_bytes == 0) {
                    printf("Pipe to server closed\n");
                    Close(i);
                    FD_CLR(i, &master);
                    continue;
                }

                // Process input from different sources
                if (i == from_input_pipe) {
                    if (!strcmp(received_msg, "STOP")) {
                        // Terminate all processes when STOP is received
                        Write(to_drone_pipe, "STOP", MAX_MSG_LEN);
                        Write(to_map_pipe, "STOP", MAX_MSG_LEN);
                        Write(to_obstacle_pipe, "STOP", MAX_MSG_LEN);
                        Write(to_target_pipe, "STOP", MAX_MSG_LEN);
                        stop_requested = true;
                        break;
                    } else if (!strcmp(received_msg, "U")) {
                        // Send drone position and velocity to input process
                        sprintf(msg_to_send, "%f,%f|%f,%f", drone_current_pos.x,
                                drone_current_pos.y,
                                drone_current_velocity.x_component,
                                drone_current_velocity.y_component);
                        Write(to_input_pipe, msg_to_send, MAX_MSG_LEN);
                    } else {
                        // Forward force commands from input to the drone
                        Write(to_drone_pipe, received_msg, MAX_MSG_LEN);
                    }

                } else if (i == from_drone_pipe) {
                    // Receive updated drone position and velocity
                    sscanf(received_msg, "%f,%f|%f,%f", &drone_current_pos.x,
                           &drone_current_pos.y,
                           &drone_current_velocity.x_component,
                           &drone_current_velocity.y_component);
                    
                    // Notify the map about the updated drone position
                    sprintf(msg_to_send, "D%f|%f", drone_current_pos.x,
                            drone_current_pos.y);
                    Write(to_map_pipe, msg_to_send, MAX_MSG_LEN);

                } else if (i == from_map_pipe) {
                    logging("INFO", received_msg);
                    
                    if (!strcmp(received_msg, "GE")) {
                        // Notify the target process to generate new targets
                        Write(to_target_pipe, "GE", MAX_MSG_LEN);
                    } else if (received_msg[0] == 'T' && received_msg[1] == 'H') {
                        // If a target is hit, inform the drone to update its tracking
                        Write(to_drone_pipe, received_msg, MAX_MSG_LEN);
                    }

                } else if (i == from_obstacles_pipe) {
                    // Forward new obstacles to both map and drone
                    Write(to_map_pipe, received_msg, MAX_MSG_LEN);
                    Write(to_drone_pipe, received_msg, MAX_MSG_LEN);

                } else if (i == from_target_pipe) {
                    // Forward new target updates to both map and drone
                    Write(to_map_pipe, received_msg, MAX_MSG_LEN);
                    Write(to_drone_pipe, received_msg, MAX_MSG_LEN);
                }
            }
        }

        // Exit the loop if a STOP signal was received
        if (stop_requested)
            break;
    }

    // Closing all pipes before terminating the server process
    Close(from_drone_pipe);
    Close(from_input_pipe);
    Close(from_map_pipe);
    Close(from_obstacles_pipe);
    Close(from_target_pipe);
    Close(to_drone_pipe);
    Close(to_map_pipe);
    Close(to_obstacle_pipe);
    Close(to_target_pipe);
    Close(to_input_pipe);

    return EXIT_SUCCESS;
}
