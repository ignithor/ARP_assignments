#include "constants.h"
#include "wrappers/wrappers.h"

// Function to spawn a new process and execute a command
static void spawn(char **exec_args) {
    if (Execvp(exec_args[0], exec_args) == -1) {
        perror("Error executing command");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    // Specifying that argc and argv are unused variables
    (void)(argc);
    (void)(argv);

    // Define an array of strings for every process to spawn
    int log_file = creat("../log/process.log", 0666);

    if (log_file < 0) {
        perror("Error creating log file");
        exit(EXIT_FAILURE);
    }
    logging("INFO", "Beginning of the master process");

    char process_names[NUM_PROCESSES][20];
    strcpy(process_names[0], "./server");
    strcpy(process_names[1], "./drone");
    strcpy(process_names[2], "./input");
    strcpy(process_names[3], "./map");
    strcpy(process_names[4], "./target");
    strcpy(process_names[5], "./obstacle");
    strcpy(process_names[6], "./watchdog");

    // Array to store child process PIDs
    pid_t child_pids[NUM_PROCESSES];

    // Array to store child PIDs as strings (excluding WD)
    char child_pids_str[NUM_PROCESSES - 1][80];

    // Arrays to contain the pids
    int server_drone[2];
    int drone_server[2];
    int server_input[2];
    int input_server[2];
    int map_server[2];
    int server_map[2];
    int target_server[2];
    int server_target[2];
    int obstacle_server[2];
    int server_obstacle[2];

    // Creating the pipes
    Pipe(server_drone);
    Pipe(drone_server);
    Pipe(server_input);
    Pipe(input_server);
    Pipe(map_server);
    Pipe(server_map);
    Pipe(target_server);
    Pipe(server_target);
    Pipe(obstacle_server);
    Pipe(server_obstacle);

    // Strings to pass pipe values as arguments
    char drone_server_str[10];
    char server_drone_str[10];
    char server_input_str[10];
    char input_server_str[10];
    char map_server_str[10];
    char server_map_str[10];
    char target_server_str[10];
    char server_target_str[10];
    char obstacle_server_str[10];
    char server_obstacle_str[10];

    for (int i = 0; i < NUM_PROCESSES; i++) {
        child_pids[i] = Fork();
        if (!child_pids[i]) {

            // Spawn the input and map process using konsole
            char *exec_args[]        = {process_names[i],
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL};
            char *konsole_arg_list[] = {
                "konsole", "-e", process_names[i], NULL, NULL, NULL, NULL};

            switch (i) {
                case 0:
                    // **Server Process Setup**
                    // Convert pipe file descriptors to strings for argument
                    // passing.
                    sprintf(drone_server_str, "%d", drone_server[0]);
                    sprintf(server_drone_str, "%d", server_drone[1]);
                    sprintf(input_server_str, "%d", input_server[0]);
                    sprintf(server_input_str, "%d", server_input[1]);
                    sprintf(map_server_str, "%d", map_server[0]);
                    sprintf(server_map_str, "%d", server_map[1]);
                    sprintf(target_server_str, "%d", target_server[0]);
                    sprintf(server_target_str, "%d", server_target[1]);
                    sprintf(obstacle_server_str, "%d", obstacle_server[0]);
                    sprintf(server_obstacle_str, "%d", server_obstacle[1]);

                    // Assign arguments for the server process
                    exec_args[1]  = drone_server_str;
                    exec_args[2]  = server_drone_str;
                    exec_args[3]  = input_server_str;
                    exec_args[4]  = server_input_str;
                    exec_args[5]  = map_server_str;
                    exec_args[6]  = server_map_str;
                    exec_args[7]  = target_server_str;
                    exec_args[8]  = server_target_str;
                    exec_args[9]  = obstacle_server_str;
                    exec_args[10] = server_obstacle_str;

                    // **Close unused pipe ends** to prevent resource leaks
                    Close(drone_server[1]); // Server only reads from drone
                    Close(server_drone[0]); // Server only writes to drone
                    Close(input_server[1]);
                    Close(server_input[0]);
                    Close(map_server[1]);
                    Close(server_map[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);

                    // Spawn the server process
                    spawn(exec_args);
                    break;

                case 1:
                    // **Drone Process Setup**
                    sprintf(server_drone_str, "%d", server_drone[0]);
                    sprintf(drone_server_str, "%d", drone_server[1]);

                    // Assign arguments for the drone process
                    exec_args[1] = server_drone_str;
                    exec_args[2] = drone_server_str;

                    // **Close unused pipe ends**
                    Close(server_drone[1]); // Drone only reads from server
                    Close(drone_server[0]); // Drone only writes to server

                    // Close all unrelated pipes to avoid interference
                    Close(server_input[0]);
                    Close(server_input[1]);
                    Close(input_server[0]);
                    Close(input_server[1]);
                    Close(map_server[0]);
                    Close(map_server[1]);
                    Close(server_map[0]);
                    Close(server_map[1]);
                    Close(target_server[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(server_target[1]);
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);

                    // Spawn the drone process
                    spawn(exec_args);
                    break;

                case 2:
                    // **Input Process Setup**
                    // Convert pipe descriptors to strings for argument passing
                    sprintf(input_server_str, "%d", input_server[1]);
                    sprintf(server_input_str, "%d", server_input[0]);

                    // Assign pipe arguments for the input process
                    konsole_arg_list[3] = input_server_str;
                    konsole_arg_list[4] = server_input_str;

                    // **Close unused pipe ends** for input process
                    Close(input_server[0]); // Input only writes to the server
                    Close(server_input[1]); // Input only reads from the server

                    // Close all unrelated pipes
                    Close(map_server[0]);
                    Close(map_server[1]);
                    Close(server_map[0]);
                    Close(server_map[1]);
                    Close(target_server[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(server_target[1]);
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);

                    // Launch the input process in a new terminal window
                    Execvp("konsole", konsole_arg_list);
                    exit(EXIT_FAILURE);
                    break;

                case 3:
                    // **Map Process Setup**
                    // Convert pipe descriptors to strings for argument passing
                    sprintf(map_server_str, "%d", map_server[1]);
                    sprintf(server_map_str, "%d", server_map[0]);

                    // Assign pipe arguments for the map process
                    konsole_arg_list[3] = map_server_str;
                    konsole_arg_list[4] = server_map_str;

                    // **Close unused pipe ends** for map process
                    Close(map_server[0]); // Map only writes to the server
                    Close(server_map[1]); // Map only reads from the server

                    // Close all unrelated pipes
                    Close(target_server[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(server_target[1]);
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);

                    // Launch the map process in a new terminal window
                    Execvp("konsole", konsole_arg_list);
                    exit(EXIT_FAILURE);
                    break;

                case 4:
                    // **Target Process Setup**
                    // Convert pipe descriptors to strings for argument passing
                    sprintf(target_server_str, "%d", target_server[1]);
                    sprintf(server_target_str, "%d", server_target[0]);

                    // Assign pipe arguments for the target process
                    exec_args[1] = target_server_str;
                    exec_args[2] = server_target_str;

                    // **Close unused pipe ends** for the target process
                    Close(target_server[0]); // Target only writes to the server
                    Close(
                        server_target[1]); // Target only reads from the server

                    // Close all unrelated pipes (Obstacle-related)
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);

                    // Spawn the target process
                    spawn(exec_args);
                    break;

                case 5:
                    // **Obstacle Process Setup**
                    // Convert pipe descriptors to strings for argument passing
                    sprintf(obstacle_server_str, "%d", obstacle_server[1]);
                    sprintf(server_obstacle_str, "%d", server_obstacle[0]);

                    // Assign pipe arguments for the obstacle process
                    exec_args[1] = obstacle_server_str;
                    exec_args[2] = server_obstacle_str;

                    // **Close unused pipe ends** for the obstacle process
                    Close(obstacle_server[0]); // Obstacle only writes to the
                                               // server
                    Close(server_obstacle[1]); // Obstacle only reads from the
                                               // server

                    // Spawn the obstacle process
                    spawn(exec_args);
                    break;
            }
            //  Spawn the last process: Watchdog (WD), which monitors all other
            //  processes
            if (i == NUM_PROCESSES - 1) {
                for (int i = 0; i < NUM_PROCESSES - 1; i++)
                    sprintf(child_pids_str[i], "%d", child_pids[i]);

                // Sending as arguments to the WD all the processes PIDs
                char *exec_args[] = {process_names[i],  child_pids_str[0],
                                     child_pids_str[1], child_pids_str[2],
                                     child_pids_str[3], child_pids_str[4],
                                     child_pids_str[5], NULL};
                spawn(exec_args);
            }
        } else {
            // **Parent Process: Close unused pipes**
            // Since pipes are duplicated for each fork, close unnecessary ones
            switch (i) {
                case 1: // **Drone has spawned**
                    Close(server_drone[0]);
                    Close(server_drone[1]);
                    Close(drone_server[0]);
                    Close(drone_server[1]);
                    break;

                case 2: // **Input has spawned**
                    Close(server_input[0]);
                    Close(server_input[1]);
                    Close(input_server[0]);
                    Close(input_server[1]);
                    break;

                case 3: // **Map has spawned**
                    Close(server_map[0]);
                    Close(server_map[1]);
                    Close(map_server[0]);
                    Close(map_server[1]);
                    break;

                case 4: // **Target has spawned**
                    Close(target_server[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(server_target[1]);
                    break;

                case 5: // **Obstacle has spawned**
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);
                    break;
            }
        }
    }

    // Print PIDs of all spawned processes
    printf("\n--- Process PIDs ---\n");
    printf("Server     PID: %d\n", child_pids[0]);
    printf("Drone      PID: %d\n", child_pids[1]);
    printf("Input GUI  PID: %d (Konsole)\n", child_pids[2]);
    printf("Map GUI    PID: %d (Konsole)\n", child_pids[3]);
    printf("Target     PID: %d\n", child_pids[4]);
    printf("Obstacle   PID: %d\n", child_pids[5]);
    printf("Watchdog   PID: %d\n", child_pids[6]);
    printf("---------------------\n\n");

    // Value for waiting for the children to terminate
    int exit_status;

    // Retrieve and display the exit status of the terminated process
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int ret = Wait(&exit_status);
        // Getting the exit status
        int status = 0;
        WEXITSTATUS(status);
        printf("Process %d terminated with code: %d\n", ret, status);
    }
    close(log_file);

    return EXIT_SUCCESS;
}
