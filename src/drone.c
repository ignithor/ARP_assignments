#include "constants.h"
#include "droneDataStructs.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"
#include <math.h>

// Computes the repulsive force exerted by a border based on the given
// parameters. The formula used is detailed in the documentation. The parameters
// can be adjusted in the configuration file.
float compute_repulsive_force(float distance, float function_scale,
                              float area_of_effect, float vel_x, float vel_y) {
    // Calculates the repulsive force based on distance, velocity, and
    // configured scaling.
    return function_scale * ((1 / distance) - (1 / area_of_effect)) *
           (1 / (distance * distance)) * sqrt(pow(vel_x, 2) + pow(vel_y, 2));
}

int main(int argc, char *argv[]) {
    // Handle watchdog signals to monitor the process
    HANDLE_WATCHDOG_SIGNALS();

    // Validate command-line arguments and extract pipe file descriptors
    int from_server_pipe, to_server_pipe;
    if (argc == 3) {
        sscanf(argv[1], "%d", &from_server_pipe);
        sscanf(argv[2], "%d", &to_server_pipe);
    } else {
        printf("Wrong number of arguments in drone\n");
        getchar();
        exit(1);
    }

    // Initialize Structs for Drone Dynamics
    // Stores the force applied to the drone from user input
    struct force drone_force = {0, 0};
    // Stores the repulsive force from nearby walls
    struct force walls = {0, 0};
    // Stores the current position of the drone
    struct pos drone_current_position = {0, 0};
    // Stores the current velocity of the drone
    struct velocity drone_current_velocity = {0, 0};
    // Stores the total forces from obstacles and targets
    struct force total_obstacles_forces = {0, 0};
    struct force total_targets_forces   = {0, 0};

    // Retrieve Parameters from Config File
    // Parameters affecting border repulsion forces
    float function_scale = get_param("drone", "function_scale");
    float area_of_effect = get_param("drone", "area_of_effect");
    float obst_of_effect = get_param("drone", "obst_of_effect");
    float targ_of_effect = get_param("drone", "targ_of_effect");

    // Drone physics parameters
    float M = get_param("drone", "mass");                // Drone mass
    float T = get_param("drone", "time_step");           // Simulation time step
    float K = get_param("drone", "viscous_coefficient"); // Drag coefficient

    // Initialize Position Variables for Time Tracking
    // prev_x and prev2_x represent past x-coordinates (for motion calculations)
    // prev_y and prev2_y represent past y-coordinates
    float prev_x, prev2_x, prev_y, prev2_y;

    // Set Initial Position from Config File
    drone_current_position.x = prev_x = prev2_x = INIT_POSE_X;
    drone_current_position.y = prev_y = prev2_y = INIT_POSE_Y;

    // Initialize Force and Velocity to Zero
    drone_force.x_component = drone_force.y_component = 0;
    drone_current_velocity.x_component = drone_current_velocity.y_component = 0;
    // Determine the update frequency for reading configuration values
    // The interval is calculated based on the reading frequency defined in
    // the config file and the simulation time step (T).
    // See input.c for a detailed explanation of this logic.
    int reading_params_interval =
        round(get_param("drone", "reading_params_interval") / T);

    // Ensure a minimum interval of 1 to prevent excessively frequent reads
    if (reading_params_interval < 1)
        reading_params_interval = 1;

    // **Initialize File Descriptor Sets for Select Call**
    fd_set master, reader;
    FD_ZERO(&master);
    FD_ZERO(&reader);
    FD_SET(from_server_pipe, &master); // Monitor the server pipe

    // Configure Select Timeout
    // Set to zero to make it as responsive as possible, keeping calculations
    // close to real-time.
    struct timeval select_timeout = {0, 0}; // Seconds = 0, Microseconds = 0

    // Buffer for sending data to the server
    char server_message[MAX_MSG_LEN];

    // Define the max file descriptor for select() syscall
    int fd = from_server_pipe;

    // Initialize Data Storage for Targets and Obstacles
    // Arrays to store the positions of detected targets and obstacles.
    struct pos targets_arr[N_TARGETS];
    struct pos obstacles_arr[N_OBSTACLES];

    // Counters to track the number of detected targets and obstacles
    int targets_num   = 0;
    int obstacles_num = 0;

    // Flag indicating if the program should terminate after receiving a STOP
    // request
    bool to_exit = false;

    while (1) {
        // Check if it's time to update parameters from the configuration file
        if (!reading_params_interval--) {
            // Read the update interval itself first
            reading_params_interval =
                round((float)get_param("drone", "reading_params_interval") / T);
            if (reading_params_interval < 1)
                reading_params_interval = 1;

            // Update physical parameters from the config file
            M              = get_param("drone", "mass");
            T              = get_param("drone", "time_step");
            K              = get_param("drone", "viscous_coefficient");
            function_scale = get_param("drone", "function_scale");
            area_of_effect = get_param("drone", "area_of_effect");
            obst_of_effect = get_param("drone", "obst_of_effect");
            targ_of_effect = get_param("drone", "targ_of_effect");

            // Log the update
            logging("INFO", "Drone has updated its parameters");
        }

        // Perform the select operation to wait for incoming data
        // Buffer to store received messages
        char received[MAX_MSG_LEN];

        // Reset the file descriptor sets
        reader = master;
        Select_wmask(fd + 1, &reader, NULL, NULL, &select_timeout);

        // Reset timeout values for the next iteration
        select_timeout.tv_sec  = 0;
        select_timeout.tv_usec = 0;

        // Process incoming data from available file descriptors
        for (int i = 0; i <= fd; i++) {
            if (FD_ISSET(i, &reader)) {
                // Read data from the available file descriptor
                int ret = Read(i, received, MAX_MSG_LEN);
                if (ret == 0) {
                    // If a pipe is closed, log a warning and remove it from
                    // monitoring
                    logging("WARN", "Pipe closed in drone");
                    Close(i);
                    FD_CLR(i, &master);
                }

                // Check for termination signal
                if (!strcmp(received, "STOP")) {
                    to_exit = true;
                    break;
                }

                // Process the received message based on its type
                switch (received[0]) {
                    case 'T':
                        // If "TH" is received, it indicates a target has been
                        // hit
                        if (received[1] == 'H') {
                            int target_index = 0;
                            float target_x = 0, target_y = 0;

                            // Extract the hit target's index and position
                            sscanf(received + 2, "|%d|%f,%f", &target_index,
                                   &target_x, &target_y);

                            // Validate the hit target's coordinates
                            if (targets_arr[target_index].x != target_x ||
                                targets_arr[target_index].y != target_y) {
                                printf("%f %f %f %f\n",
                                       targets_arr[target_index].x,
                                       targets_arr[target_index].y, target_x,
                                       target_y);
                                fflush(stdout);
                                logging("ERROR",
                                        "Mismatched target and array in drone");
                            } else {
                                // Remove the target from the array
                                remove_target(target_index, targets_arr,
                                              targets_num);
                                targets_num--; // Decrease the target count
                            }
                        } else {
                            // If "T" is received, new targets have been
                            // generated

                            // logging("INFO", "vvvvvvvvDRONE-TARGvvvvvvv");
                            logging("INFO", "Processing new target data...");
                            tokenization(targets_arr, received, &targets_num);
                            sprintf(server_message, "Total targets updated: %d", targets_num);
                            logging("INFO", "End new taget data");
                        }
                        break;

                    case 'O':  
                        // If "O" is received, new obstacles have been generated  
                        logging("INFO", "Processing new obstacle data...");
                        tokenization(obstacles_arr, received, &obstacles_num);
                        logging("INFO", "End new obstacle data...");
                        break;

                    default:
                        // If none of the above, assume the message contains
                        // force components
                        sscanf(received, "%f|%f", &drone_force.x_component,
                               &drone_force.y_component);
                        break;
                }
            }
        }

        // If the exit flag is set, terminate the loop
        if (to_exit)
            break;

        // Reset total obstacle repulsive forces
        total_obstacles_forces.x_component = 0;
        total_obstacles_forces.y_component = 0;

        // Calculate repulsive forces from each obstacle
        for (int i = 0; i < obstacles_num; i++) {
            float x_dist   = obstacles_arr[i].x - prev_x;
            float y_dist   = obstacles_arr[i].y - prev_y;
            float distance = sqrt(pow(x_dist, 2) + pow(y_dist, 2));

            // Apply repulsive force only if within effect range and not too
            // close
            if (distance < obst_of_effect && distance > 1) {
                // Compute the magnitude of the repulsive force
                double force_magnitude = -compute_repulsive_force(
                    distance, 10000, obst_of_effect,
                    drone_current_velocity.x_component,
                    drone_current_velocity.y_component);

                // Compute the direction of the repulsive force
                double angle = atan2(y_dist, x_dist);

                // Apply force in the computed direction
                total_obstacles_forces.x_component +=
                    cos(angle) * force_magnitude;
                total_obstacles_forces.y_component +=
                    sin(angle) * force_magnitude;

                // Cap the force to prevent extreme values
                total_obstacles_forces.x_component = fmax(
                    fmin(total_obstacles_forces.x_component, MAX_OBST_FORCES),
                    -MAX_OBST_FORCES);
                total_obstacles_forces.y_component = fmax(
                    fmin(total_obstacles_forces.y_component, MAX_OBST_FORCES),
                    -MAX_OBST_FORCES);
            }
        }

        // Compute attractive forces from targets
        total_targets_forces.x_component = 0;
        total_targets_forces.y_component = 0;

        for (int i = 0; i < targets_num; i++) {
            // Compute distance to the target
            float x_dist   = targets_arr[i].x - prev_x;
            float y_dist   = targets_arr[i].y - prev_y;
            float distance = sqrt(pow(x_dist, 2) + pow(y_dist, 2));

            // Apply attractive force if within the target's effect range
            if (distance < targ_of_effect) {
                // Compute force magnitude
                double force_magnitude =
                    compute_repulsive_force(distance, 1000, targ_of_effect,
                                            drone_current_velocity.x_component,
                                            drone_current_velocity.y_component);

                // Compute force direction
                double angle = atan2(y_dist, x_dist);

                // Apply force in the computed direction
                total_targets_forces.x_component +=
                    cos(angle) * force_magnitude;
                total_targets_forces.y_component +=
                    sin(angle) * force_magnitude;

                // Cap forces to prevent extreme values
                total_targets_forces.x_component = fmax(
                    fmin(total_targets_forces.x_component, MAX_TARG_FORCES),
                    -MAX_TARG_FORCES);
                total_targets_forces.y_component = fmax(
                    fmin(total_targets_forces.y_component, MAX_TARG_FORCES),
                    -MAX_TARG_FORCES);
            }
        }

        // Compute repulsive force from simulation boundaries
        // The function effect is applied only when within 'area_of_effect' from
        // a boundary
        if (prev_x < area_of_effect) {
            walls.x_component =
                compute_repulsive_force(prev_x, function_scale, area_of_effect,
                                        drone_current_velocity.x_component,
                                        drone_current_velocity.y_component);
        } else if (prev_x > SIMULATION_WIDTH - area_of_effect) {
            walls.x_component = -compute_repulsive_force(
                SIMULATION_WIDTH - prev_x, function_scale, area_of_effect,
                drone_current_velocity.x_component,
                drone_current_velocity.y_component);
        } else {
            walls.x_component =
                0; // No force applied when sufficiently far from boundaries
        }

        // Compute repulsive force from top and bottom boundaries
        if (prev_y < area_of_effect) {
            walls.y_component =
                compute_repulsive_force(prev_y, function_scale, area_of_effect,
                                        drone_current_velocity.x_component,
                                        drone_current_velocity.y_component);
        } else if (prev_y > SIMULATION_HEIGHT - area_of_effect) {
            walls.y_component = -compute_repulsive_force(
                SIMULATION_HEIGHT - prev_y, function_scale, area_of_effect,
                drone_current_velocity.x_component,
                drone_current_velocity.y_component);
        } else {
            walls.y_component = 0;
        }

        // Calculate the new position of the drone using the given physics
        // formula. The same calculation is applied to both x and y axes.

        // Prevent small floating-point values from preventing the velocity from
        // reaching zero. A threshold is applied only when no external forces
        // are acting on the drone.
        if (fabs(drone_current_velocity.x_component) < ZERO_THRESHOLD &&
            walls.x_component == 0 && drone_force.x_component == 0 &&
            total_obstacles_forces.x_component == 0 &&
            total_targets_forces.x_component == 0) {
            drone_current_position.x = prev_x;
        } else {
            drone_current_position.x =
                (walls.x_component + drone_force.x_component +
                 total_obstacles_forces.x_component +
                 total_targets_forces.x_component -
                 (M / (T * T)) * (prev2_x - 2 * prev_x) + (K / T) * prev_x) /
                ((M / (T * T)) + K / T);
        }

        if (fabs(drone_current_velocity.y_component) < ZERO_THRESHOLD &&
            walls.y_component == 0 && drone_force.y_component == 0 &&
            total_obstacles_forces.y_component == 0 &&
            total_targets_forces.y_component == 0) {
            drone_current_position.y = prev_y;
        } else {
            drone_current_position.y =
                (walls.y_component + drone_force.y_component +
                 total_obstacles_forces.y_component +
                 total_targets_forces.y_component -
                 (M / (T * T)) * (prev2_y - 2 * prev_y) + (K / T) * prev_y) /
                ((M / (T * T)) + K / T);
        }

        // Enforce simulation boundaries to prevent the drone from escaping the
        // defined area. This also ensures that the repulsive force calculations
        // remain valid. If the force is set too high, the next position might
        // completely bypass the border effect. Special handling is applied near
        // position 0 to avoid infinite force due to mathematical asymptotes.
        if (drone_current_position.x > SIMULATION_WIDTH)
            drone_current_position.x = SIMULATION_WIDTH - 1;
        else if (drone_current_position.x < 0)
            drone_current_position.x = 1;

        if (drone_current_position.y > SIMULATION_HEIGHT)
            drone_current_position.y = SIMULATION_HEIGHT - 1;
        else if (drone_current_position.y < 0)
            drone_current_position.y = 1;

        // Compute the current velocity using the change in position over the
        // time step. This calculation ensures that the displayed velocity is as
        // up-to-date as possible. There is a slight delay of one iteration in
        // detecting velocity zero, but this does not significantly affect the
        // simulation.
        drone_current_velocity.x_component =
            (drone_current_position.x - prev_x) / T;
        drone_current_velocity.y_component =
            (drone_current_position.y - prev_y) / T;

        // Update time-dependent position variables for the next iteration.
        prev2_x = prev_x;
        prev_x  = drone_current_position.x;

        prev2_y = prev_y;
        prev_y  = drone_current_position.y;

        // Send the updated position and velocity to the server.
        // This allows the input process to display it in the ncurses interface
        // and the map to render the drone's position on screen.
        sprintf(server_message, "%f,%f|%f,%f", drone_current_position.x,
                drone_current_position.y, drone_current_velocity.x_component,
                drone_current_velocity.y_component);
        Write(to_server_pipe, server_message, MAX_MSG_LEN);

        // Sleep for the configured time step before recalculating the position.
        // The sleep duration is converted from seconds to microseconds.
        usleep(1000000 * T);
    }

    // Cleanup: Close the pipe before exiting.
    Close(to_server_pipe);
    return 0;
}