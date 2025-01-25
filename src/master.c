#include "constants.h"
#include "wrapFuncs/wrapFunc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Function to spawn the processes
static void spawn(char **arg_list) {
    Execvp(arg_list[0], arg_list);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    // Specifying that argc and argv are unused variables
    (void)(argc);
    (void)(argv);

    // Define an array of strings for every process to spawn
    char programs[NUM_PROCESSES][20];
    strcpy(programs[0], "./server");
    strcpy(programs[1], "./drone");
    strcpy(programs[2], "./input");
    strcpy(programs[3], "./map");
    strcpy(programs[4], "./target");
    strcpy(programs[5], "./obstacle");
    strcpy(programs[6], "./watchdog");

    // Pids for all children
    pid_t child[NUM_PROCESSES];

    // String to contain all che children pids (except WD)
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

    // strings to pass pipe values as args
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
        child[i] = Fork();
        if (!child[i]) {

            // Spawn the input and map process using konsole
            char *arg_list[] = {programs[i], NULL, NULL, NULL, NULL, NULL,
                                NULL,        NULL, NULL, NULL, NULL, NULL};
            char *konsole_arg_list[] = {"konsole", "-e", programs[i], NULL,
                                        NULL,      NULL, NULL};

            switch (i) {
                case 0:
                    // Server
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
                    arg_list[1]  = drone_server_str;
                    arg_list[2]  = server_drone_str;
                    arg_list[3]  = input_server_str;
                    arg_list[4]  = server_input_str;
                    arg_list[5]  = map_server_str;
                    arg_list[6]  = server_map_str;
                    arg_list[7]  = target_server_str;
                    arg_list[8]  = server_target_str;
                    arg_list[9]  = obstacle_server_str;
                    arg_list[10] = server_obstacle_str;
                    Close(drone_server[1]);
                    Close(server_drone[0]);
                    Close(input_server[1]);
                    Close(server_input[0]);
                    Close(map_server[1]);
                    Close(server_map[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    spawn(arg_list);
                    break;
                case 1:
                    // Drone
                    sprintf(server_drone_str, "%d", server_drone[0]);
                    sprintf(drone_server_str, "%d", drone_server[1]);
                    arg_list[1] = server_drone_str;
                    arg_list[2] = drone_server_str;
                    Close(server_drone[1]);
                    Close(drone_server[0]);

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
                    spawn(arg_list);
                    break;
                case 2:
                    // Input
                    sprintf(input_server_str, "%d", input_server[1]);
                    sprintf(server_input_str, "%d", server_input[0]);
                    konsole_arg_list[3] = input_server_str;
                    konsole_arg_list[4] = server_input_str;
                    Close(input_server[0]);
                    Close(server_input[1]);

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

                    Execvp("konsole", konsole_arg_list);
                    exit(EXIT_FAILURE);
                    break;
                case 3:
                    // Map
                    sprintf(map_server_str, "%d", map_server[1]);
                    sprintf(server_map_str, "%d", server_map[0]);
                    konsole_arg_list[3] = map_server_str;
                    konsole_arg_list[4] = server_map_str;
                    Close(map_server[0]);
                    Close(server_map[1]);

                    Close(target_server[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(server_target[1]);
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);

                    Execvp("konsole", konsole_arg_list);
                    exit(EXIT_FAILURE);
                    break;
                case 4:
                    // Target
                    sprintf(target_server_str, "%d", target_server[1]);
                    sprintf(server_target_str, "%d", server_target[0]);
                    arg_list[1] = target_server_str;
                    arg_list[2] = server_target_str;
                    Close(target_server[0]);
                    Close(server_target[1]);

                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);

                    spawn(arg_list);
                    break;
                case 5:
                    // Obstacle
                    sprintf(obstacle_server_str, "%d", obstacle_server[1]);
                    sprintf(server_obstacle_str, "%d", server_obstacle[0]);
                    arg_list[1] = obstacle_server_str;
                    arg_list[2] = server_obstacle_str;
                    Close(obstacle_server[0]);
                    Close(server_obstacle[1]);

                    spawn(arg_list);
                    break;
            }
            // spawn the last program, so the WD, which needs all the processes
            // PIDs
            if (i == NUM_PROCESSES - 1) {
                for (int i = 0; i < NUM_PROCESSES - 1; i++)
                    sprintf(child_pids_str[i], "%d", child[i]);

                // Sending as arguments to the WD all the processes PIDs
                char *arg_list[] = {programs[i],       child_pids_str[0],
                                    child_pids_str[1], child_pids_str[2],
                                    child_pids_str[3], child_pids_str[4],
                                    child_pids_str[5], NULL};
                spawn(arg_list);
            }
        } else {
            // If we are in the father we need to close all the unused pipes
            // since they are duplicated every time
            switch (i) {
                case 1:
                    // Drone has spawned
                    Close(server_drone[0]);
                    Close(server_drone[1]);
                    Close(drone_server[0]);
                    Close(drone_server[1]);
                    break;
                case 2:
                    // Input has spawned
                    Close(server_input[0]);
                    Close(server_input[1]);
                    Close(input_server[0]);
                    Close(input_server[1]);
                    break;
                case 3:
                    // Map has spawned
                    Close(server_map[0]);
                    Close(server_map[1]);
                    Close(map_server[0]);
                    Close(map_server[1]);
                    break;
                case 4:
                    // Target has spawned
                    Close(target_server[0]);
                    Close(target_server[1]);
                    Close(server_target[0]);
                    Close(server_target[1]);
                    break;
                case 5:
                    // Obstacle has spawned
                    Close(obstacle_server[0]);
                    Close(obstacle_server[1]);
                    Close(server_obstacle[0]);
                    Close(server_obstacle[1]);
                    break;
            }
        }
    }

    // Printing the pids
    printf("Server pid is %d\n", child[0]);
    printf("Drone pid is %d\n", child[1]);
    printf("Konsole of Input pid is %d\n", child[2]);
    printf("Konsole of Map pid is %d\n", child[3]);
    printf("Target pid is %d\n", child[4]);
    printf("Obstacle pid is %d\n", child[5]);
    printf("Watchdog pid is %d\n", child[6]);

    // Value for waiting for the children to terminate
    int res;

    // Wait for all direct children to terminate. Map and the konsole on which
    // it runs on are not direct childs of the master process but of the server
    // one so they will not return here
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int ret = Wait(&res);
        // Getting the exit status
        int status = 0;
        WEXITSTATUS(status);
        printf("Process %d terminated with code: %d\n", ret, status);
    }

    return EXIT_SUCCESS;
}
