#include "constants.h"
#include "dataStructs.h"
#include "utils/utils.h"
#include "wrapFuncs/wrapFunc.h"
#include <math.h>

// WD pid
pid_t WD_pid;

// This function returns the border effect given the general
// function given in the docs folder of the project. All the parameters can be
// modified from the configuration file.
float repulsive_force(float distance, float function_scale,
                      float area_of_effect, float vel_x, float vel_y) {
    // return function_scale * (area_of_effect - distance) / (distance /
    // function_scale);
    return function_scale * ((1 / distance) - (1 / area_of_effect)) *
           (1 / (distance * distance)) * sqrt(pow(vel_x, 2) + pow(vel_y, 2));
}

int main(int argc, char *argv[]) {
    // Macro to handle the watchdog signals
    HANDLE_WATCHDOG_SIGNALS();

    // Specifying that argc and argv are unused variables
    int from_server_pipe, to_server_pipe;

    if (argc == 3) {
        sscanf(argv[1], "%d", &from_server_pipe);
        sscanf(argv[2], "%d", &to_server_pipe);
    } else {
        printf("Wrong number of arguments in drone\n");
        getchar();
        exit(1);
    }

    // Initializing structs to store data gotten and given by this process
    // drone_force as every other of these structs has a x and a y component
    // Drone force is the force applied to the drone by the input
    struct force drone_force;
    // walls is the force acting on the drone due to the close distance to
    // the walls
    struct force walls = {0, 0};
    // drone_current_position stores the current position of the drone
    struct pos drone_current_position = {0, 0};
    // drone_current_velocity stores the curretn velocity of the drone
    struct velocity drone_current_velocity = {0, 0};

    struct force total_obstacles_forces = {0};
    struct force total_targets_forces   = {0};

    // Read the parameters for the border effect
    // Function scale determines the slope of the function while
    // area of effect determines for how many meters will the border repel the
    // object
    float function_scale = get_param("drone", "function_scale");
    float area_of_effect = get_param("drone", "area_of_effect");
    float obst_of_effect = get_param("drone", "obst_of_effect");
    float targ_of_effect = get_param("drone", "targ_of_effect");

    // Read a first time from the paramter file to set the constants
    float M = get_param("drone", "mass");
    float T = get_param("drone", "time_step");
    float K = get_param("drone", "viscous_coefficient");

    // Initializing the variables that will store the position at time
    // t-1 and t-2. So xt_1 means x(t-1) and xt_2 x(t-2). The same goes
    // for y variable
    float xt_1, xt_2;
    float yt_1, yt_2;

    // Setting the structs declared above with defauls values
    // Initializing the position of the drone with the parameters specified
    // in the parameters file
    drone_current_position.x = xt_1 = xt_2 = get_param("drone", "init_pos_x");
    drone_current_position.y = yt_1 = yt_2 = get_param("drone", "init_pos_y");

    // The force and the velocity applied to the drone at t = 0 is 0
    drone_force.x_component            = 0;
    drone_force.y_component            = 0;
    drone_current_velocity.x_component = 0;
    drone_current_velocity.y_component = 0;

    // Here the number of cycles to wait before reading again from file is
    // calculated after reading from the paramaters file. To have a better
    // explaination of what's happening in these lines look at the comment of
    // reading_params_interval inside the input.c file
    int reading_params_interval =
        round(get_param("drone", "reading_params_interval") / T);
    // If the counter is less than 1 is set back to 1
    if (reading_params_interval < 1)
        reading_params_interval = 1;

    // Setting select parameters
    fd_set master;
    fd_set reader;
    FD_ZERO(&master);
    FD_ZERO(&reader);

    FD_SET(from_server_pipe, &master);

    // Setting the sleep time to 0 because this has to be very reactive to keep
    // the calculations as real time as possible
    struct timeval select_timeout;
    select_timeout.tv_sec  = 0;
    select_timeout.tv_usec = 0;

    // to_send is the auxiliary buffer on where to save data before sending to
    // the server
    char to_send[MAX_MSG_LEN];

    // max_fd used for the select syscall
    int fd = from_server_pipe;

    // Arrays used to store targets and obstacles in real world coordinates
    struct pos targets_arr[N_TARGETS];
    struct pos obstacles_arr[N_OBSTACLES];

    // Counters to keep track of the dimension of the arrays
    int targets_num   = 0;
    int obstacles_num = 0;

    // boolean that indicates whether the program has to be terminated after a
    // STOP request
    bool to_exit = false;

    while (1) {
        // If reading_params_interval is equal to 0 is time to read again from
        // the parameters file
        if (!reading_params_interval--) {
            // The first parameter to be read is the reading_params_interval
            // itself.
            reading_params_interval =
                round((float)get_param("drone", "reading_params_interval") / T);
            // If the counter is less than 1 is set back to 1
            if (reading_params_interval < 1)
                reading_params_interval = 1;

            // Then all the other phisic parameters are read
            M              = get_param("drone", "mass");
            T              = get_param("drone", "time_step");
            K              = get_param("drone", "viscous_coefficient");
            function_scale = get_param("drone", "function_scale");
            area_of_effect = get_param("drone", "area_of_effect");
            obst_of_effect = get_param("drone", "obst_of_effect");
            targ_of_effect = get_param("drone", "targ_of_effect");

            // Logging
            logging(LOG_INFO, "Drone has updated its parameters");
        }
        // The semaphore is taken in order to read the force components as
        // given by the user in the input process
        // get drone current x and y

        /// Perform the select
        // Auxiliary array where to save the received string
        char received[MAX_MSG_LEN];

        // resetting the fd_sets
        reader = master;
        Select_wmask(fd + 1, &reader, NULL, NULL, &select_timeout);
        // Resetting timouts
        select_timeout.tv_sec  = 0;
        select_timeout.tv_usec = 0;
        for (int i = 0; i <= fd; i++) {
            if (FD_ISSET(i, &reader)) {
                // Receiving data from the available sets
                int ret = Read(i, received, MAX_MSG_LEN);
                if (ret == 0) {
                    // If a pipe gets closed then notify and remove it from
                    // master
                    logging(LOG_WARN, "Pipe closed in drone");
                    Close(i);
                    FD_CLR(i, &master);
                }
                // If STOP is received then it's time to quit
                if (!strcmp(received, "STOP")) {
                    to_exit = true;
                    break;
                }
                switch (received[0]) {
                    case 'T':
                        // if TH is received that indicates a target hit
                        if (received[1] == 'H') {
                            int target_index = 0;
                            float target_x   = 0;
                            float target_y   = 0;
                            // Identify the hit target
                            sscanf(received + 2, "|%d|%f,%f", &target_index,
                                   &target_x, &target_y);
                            // If the indexes and the positions of the target
                            // don't match it means that there was an error in
                            // the creation of the messages, so notify
                            if (targets_arr[target_index].x != target_x ||
                                targets_arr[target_index].y != target_y) {
                                printf("%f %f %f %f\n",
                                       targets_arr[target_index].x,
                                       targets_arr[target_index].y, target_x,
                                       target_y);
                                fflush(stdout);
                                logging(LOG_ERROR,
                                        "Mismatched target and array in drone");
                            } else {
                                // If index and positions match then remove it
                                // from the array
                                remove_target(target_index, targets_arr,
                                              targets_num);
                                // Decrease the total number of targets
                                targets_num--;
                            }
                        } else {
                            // If only T is received then new targets have been
                            // produced
                            logging(LOG_INFO, "vvvvvvvvDRONE-TARGvvvvvvv");
                            tokenization(targets_arr, received, &targets_num);
                            logging(LOG_INFO, "^^^^^^^^DRONE-TARG^^^^^^^");
                        }
                        break;
                    case 'O':
                        // If O then new obstacles have been produced
                        logging(LOG_INFO, "vvvvvvvvDRONE-OBSTvvvvvvv");
                        tokenization(obstacles_arr, received, &obstacles_num);
                        logging(LOG_INFO, "^^^^^^^^DRONE-OBST^^^^^^^");
                        break;
                    default:
                        // If none of the above then the input has sent the
                        // components of the force applied to the drone
                        sscanf(received, "%f|%f", &drone_force.x_component,
                               &drone_force.y_component);
                        break;
                }
            }
        }
        // If the exit message has been sent then quit the loop
        if (to_exit)
            break;

        // Calculating repulsive force for every obstacle
        total_obstacles_forces.x_component = 0;
        total_obstacles_forces.y_component = 0;
        for (int i = 0; i < obstacles_num; i++) {
            float distance = sqrt(pow(obstacles_arr[i].x - xt_1, 2) +
                                  pow(obstacles_arr[i].y - yt_1, 2));
            // If it's quite close but not too much then apply the force.
            if (distance < obst_of_effect && distance > 1) {
                double x_distance = obstacles_arr[i].x - xt_1;
                double y_distance = obstacles_arr[i].y - yt_1;

                // Compute the magnitude of the repulsive force
                double force =
                    -repulsive_force(distance, 10000, obst_of_effect,
                                     drone_current_velocity.x_component,
                                     drone_current_velocity.y_component);

                // Compute the direction of the repulsive force
                double angle = atan2(y_distance, x_distance);

                // Add the force to the accumulation variable taking into
                // consideration the direction
                total_obstacles_forces.x_component += cos(angle) * force;
                total_obstacles_forces.y_component += sin(angle) * force;

                // Cap the force at a certain threshold. This paramater is not
                // available in the configuartion file since it can be
                // particularily distructive if set incorrectly
                if (total_obstacles_forces.x_component > MAX_OBST_FORCES)
                    total_obstacles_forces.x_component = MAX_OBST_FORCES;
                if (total_obstacles_forces.x_component < -MAX_OBST_FORCES)
                    total_obstacles_forces.x_component = -MAX_OBST_FORCES;
                if (total_obstacles_forces.y_component > MAX_OBST_FORCES)
                    total_obstacles_forces.y_component = MAX_OBST_FORCES;
                if (total_obstacles_forces.y_component < -MAX_OBST_FORCES)
                    total_obstacles_forces.y_component = -MAX_OBST_FORCES;
            }
        }

        // Calculating repulsive force for every target
        total_targets_forces.x_component = 0;
        total_targets_forces.y_component = 0;
        for (int i = 0; i < targets_num; i++) {
            // Calculating distance
            float distance = sqrt(pow(targets_arr[i].x - xt_1, 2) +
                                  pow(targets_arr[i].y - yt_1, 2));
            // If close enough to the target apply attractive forces
            if (distance < targ_of_effect) {
                double x_distance = targets_arr[i].x - xt_1;
                double y_distance = targets_arr[i].y - yt_1;
                double force =
                    repulsive_force(distance, 1000, targ_of_effect,
                                    drone_current_velocity.x_component,
                                    drone_current_velocity.y_component);
                // Calculate direction
                double angle = atan2(y_distance, x_distance);
                total_targets_forces.x_component += cos(angle) * force;
                total_targets_forces.y_component += sin(angle) * force;
                // Cap the combined forces from the targets
                if (total_targets_forces.x_component > MAX_TARG_FORCES)
                    total_targets_forces.x_component = MAX_TARG_FORCES;
                if (total_targets_forces.x_component < -MAX_TARG_FORCES)
                    total_targets_forces.x_component = -MAX_TARG_FORCES;
                if (total_targets_forces.y_component > MAX_TARG_FORCES)
                    total_targets_forces.y_component = MAX_TARG_FORCES;
                if (total_targets_forces.y_component < -MAX_TARG_FORCES)
                    total_targets_forces.y_component = -MAX_TARG_FORCES;
            }
        }

        // Calculating repulsive force from the sides.
        // Note that in the docs the image of the function is provided and
        // it shows that only after 'area_of_effect' in the x axis it will
        // start to work. This explains the constraint in the if. So if xt_1
        // is less than 'area_of_effect' distance from the any wall in the x
        // axis it will be affected by the force
        if (xt_1 < area_of_effect) {
            walls.x_component =
                repulsive_force(xt_1, function_scale, area_of_effect,
                                drone_current_velocity.x_component,
                                drone_current_velocity.y_component);
            // In the following if the right edge is checked
        } else if (xt_1 > SIMULATION_WIDTH - area_of_effect) {
            walls.x_component = -repulsive_force(
                SIMULATION_WIDTH - xt_1, function_scale, area_of_effect,
                drone_current_velocity.x_component,
                drone_current_velocity.y_component);
        } else {
            // Otherwise set it to 0
            walls.x_component = 0;
        }

        // Here the same calculation for the y axis is performed
        if (yt_1 < area_of_effect) {
            walls.y_component =
                repulsive_force(yt_1, function_scale, area_of_effect,
                                drone_current_velocity.x_component,
                                drone_current_velocity.y_component);
            // In the following if the bottom edge is checked
        } else if (yt_1 > SIMULATION_HEIGHT - area_of_effect) {
            walls.y_component = -repulsive_force(
                SIMULATION_HEIGHT - yt_1, function_scale, area_of_effect,
                drone_current_velocity.x_component,
                drone_current_velocity.y_component);
        } else {
            walls.y_component = 0;
        }

        // Here the current position of the drone is calculated using
        // the provided formula. The x and y components are calculated
        // using the same formula

        // The current velocity is calculated by using floats and there is
        // the possibility that  when this get too small they don't reach 0,
        // so a threshold under which the value is set to zero is set. This
        // threshold has to came into play only when there is no repulsive
        // force from the walls and no force is applied by the user, so when
        // the drone is decelerating only by viscous coefficient. The same
        // applies for the x and y components. It's not possible to directly
        // set velocity to 0 so instead the current position is set equal to
        // the previous one

        if (drone_current_velocity.x_component < ZERO_THRESHOLD &&
            drone_current_velocity.x_component > -ZERO_THRESHOLD &&
            walls.x_component == 0 && drone_force.x_component == 0 &&
            total_obstacles_forces.x_component == 0 &&
            total_targets_forces.x_component == 0)
            drone_current_position.x = xt_1;
        else {
            drone_current_position.x =
                (walls.x_component + drone_force.x_component +
                 total_obstacles_forces.x_component +
                 total_targets_forces.x_component -
                 (M / (T * T)) * (xt_2 - 2 * xt_1) + (K / T) * xt_1) /
                ((M / (T * T)) + K / T);
        }

        if (drone_current_velocity.y_component < ZERO_THRESHOLD &&
            drone_current_velocity.y_component > -ZERO_THRESHOLD &&
            walls.y_component == 0 && drone_force.y_component == 0 &&
            total_obstacles_forces.y_component == 0 &&
            total_targets_forces.y_component == 0)
            drone_current_position.y = yt_1;
        else {
            drone_current_position.y =
                (walls.y_component + drone_force.y_component +
                 total_obstacles_forces.y_component +
                 total_targets_forces.y_component -
                 (M / (T * T)) * (yt_2 - 2 * yt_1) + (K / T) * yt_1) /
                ((M / (T * T)) + K / T);
        }

        // These ifs enforce the boundary of the simulation. These are
        // needed because if the force is set too high it's possible that
        // the next calculated x or y coordinate will completely jump over
        // the border effect. It's also important to notice that, for
        // example when the position is less than 0 it's then set at 1. This
        // to overcome the infinite force that is given by the 0 position
        // because the function that models the repulsive force of the wall
        // has an asymptote in 0. The same goes for all the other walls.
        if (drone_current_position.x > SIMULATION_WIDTH)
            drone_current_position.x = SIMULATION_WIDTH - 1;
        else if (drone_current_position.x < 0)
            drone_current_position.x = 1;
        if (drone_current_position.y < 0)
            drone_current_position.y = 1;
        else if (drone_current_position.y > SIMULATION_HEIGHT)
            drone_current_position.y = 1;

        // The current velocity is calculated by dividing the
        // difference of position by the time step between the calculations
        // both for the x and y axis. This calculation is done here to
        // always have the most updated velocity calculation in the input
        // Konsole. It delays by one iteration the check of velocity zero,
        // but it's not parcituculary detrimenting to the simulation
        drone_current_velocity.x_component =
            (drone_current_position.x - xt_1) / T;
        drone_current_velocity.y_component =
            (drone_current_position.y - yt_1) / T;

        // Here the time dependant values are updated
        // Every iteration a time step T is elapsed so:
        xt_2 = xt_1;
        xt_1 = drone_current_position.x;

        // This is done also for the y axis
        yt_2 = yt_1;
        yt_1 = drone_current_position.y;

        // The calculated position is written in shared memory in order to
        // allow the input process to correctly display it in the ncurses
        // interface, and the map to show the drone on screen. To do that
        // firstly the semaphore needs to be taken

        // position and velocity
        sprintf(to_send, "%f,%f|%f,%f", drone_current_position.x,
                drone_current_position.y, drone_current_velocity.x_component,
                drone_current_velocity.y_component);
        Write(to_server_pipe, to_send, MAX_MSG_LEN);

        // The process needs to wait T seconds before computing again the
        // position as specified in the paramaters file. Here usleep needs
        // the amount to sleep for in microsecons. So 1 second in
        // microseconds is given and then multiplied by T to get the correct
        // amount of microseconds to sleep for.
        usleep(1000000 * T);
    }

    // Cleaning up
    Close(to_server_pipe);
    return 0;
}
