#include "constants.h"
#include "droneDataStructs.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"
#include <math.h>
#include <time.h>
/* 
 * Global variables to track screen positions of targets and obstacles.
 * Used to check collisions or overlap with the drone.
 */
int target_obstacles_screen_position[N_TARGETS + N_OBSTACLES][2];
int tosp_top = -1; // Tracks the top index of the position array stack.

// Buffer for logging event reasons.
char event_reason[50] = "";

// Functions
/* 
 * Creates the map window for the simulation.
 * Draws a border around the specified dimensions.
 */
WINDOW *create_map_win(int height, int width, int starty, int startx) {
    WINDOW *local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0); // Draws a border around the window.
    return local_win;
}

/*
 * Clears and destroys a specified map window.
 * Useful for refreshing or resizing the map window dynamically.
 */
void destroy_map_win(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); // Removes the window border.
    wrefresh(local_win); // Refreshes the display.
    delwin(local_win);   // Deletes the window.
}

/* 
 * Checks if a given position (x, y) overlaps with the drone or other targets/obstacles.
 * Also ensures the position is within the valid display boundaries.
 */
bool is_overlapping(int y, int x, int *drone_y, int *drone_x) {
    if (y < 1 || x < 1 || y > LINES - 3 || x > COLS - 2) return true; // Out of bounds.

    // Check if it overlaps with the drone's position.
    if (drone_y != NULL && drone_x != NULL && *drone_y == y && *drone_x == x) return true;

    // Check for overlap with targets or obstacles.
    for (int i = 0; i <= tosp_top; i++) {
        if (y == target_obstacles_screen_position[i][0] && x == target_obstacles_screen_position[i][1]) return true;
    }
    return false;
}

/* 
 * Finds a valid position around a given (old_y, old_x).
 * Expands outward in a square pattern until a valid spot is found.
 */
void find_spot(int *old_y, int *old_x, int drone_y, int drone_x) {
    for (int index = 1;; index++) { // Expands the search radius indefinitely.
        int x = *old_x, y = *old_y;


        // Check the row above.
        y = *old_y - index;
        for (x = *old_x - index; x <= *old_x + index; x++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // Check the row below.
        y = *old_y + index;
        for (x = *old_x - index; x <= *old_x + index; x++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // Check the column to the left.
        x = *old_x - index;
        for (y = *old_y - index + 1; y <= *old_y + index - 1; y++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // Check the column to the left.
        x = *old_x - index;
        for (y = *old_y - index + 1; y <= *old_y + index - 1; y++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // If the index grows too large, assume no valid position is available.
        if (index > 100) {
            logging("ERROR", "Unable to find a valid position for map display.");
            exit(EXIT_FAILURE);
        }
    }
}

// Main function
/*
 * Entry point for the map display program.
 * Handles setup, communication with the server, and rendering the simulation.
 */
int main(int argc, char *argv[]) {
    HANDLE_WATCHDOG_SIGNALS(); // Initialize watchdog signals for safety.

    int to_server, from_server; // Pipes for inter-process communication.
    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server);     // Read the server's write pipe.
        sscanf(argv[2], "%d", &from_server);   // Read the server's read pipe.
    } else {
        printf("Invalid number of arguments. Expected 2 pipes.\n");
        getchar();
        exit(1);
    }

    // Score tracking.
    int score = 0, score_increment = 0;

    // Time-related variables.
    time_t start_time = 0, last_target_reached_time = 0;

    // Setup FIFO communication for watchdog.
    Mkfifo(FIFO1_PATH, 0666);
    int fd = Open(FIFO1_PATH, O_WRONLY);
    char map_pid_str[10];
    sprintf(map_pid_str, "%d", getpid());
    Write(fd, map_pid_str, strlen(map_pid_str) + 1);
    Close(fd);
    // Drone position and other entities.
    struct pos drone_pos = {0, 0};
    struct pos targets_pos[N_TARGETS];
    struct pos obstacles_pos[N_OBSTACLES];
    int target_num = 0, obstacles_num = 0;

// Setup ncurses for GUI rendering.
    initscr();
    cbreak();  // Disable line buffering.
    curs_set(0); // Hide the cursor.
    start_color(); // Enable colors.
    use_default_colors();
    init_pair(1, COLOR_BLUE, -1); // Drone color.
    init_pair(2, COLOR_RED, -1);  // Obstacle color.
    init_pair(3, COLOR_GREEN, -1); // Target color.

// Create the map window.
    WINDOW *map_window = create_map_win(getmaxy(stdscr) - 2, getmaxx(stdscr), 1, 0);

        char received[MAX_MSG_LEN]; // Buffer for incoming messages.
        fd_set master, reader;
        struct timeval select_timeout = {5, 0}; // Timeout for the select() syscall.

        FD_ZERO(&master);
        FD_SET(from_server, &master);
    // Monitor for incoming data.
    
    while (1) {
        // resetting the fd_set
        reader = master;
        int ret;
        do {
            // Signals like SIGWINCH (used by ncurses for window resizing) are not ignored 
            // to ensure proper handling. Ignoring or resetting them would prevent the GUI 
            // from resizing correctly.
            ret = Select(from_server + 1, &reader, NULL, NULL, &select_timeout);

            // Restart Select if interrupted by a signal and SA_RESTART didn't handle it.

        } while (ret == -1);
        // Resetting the timeout
        select_timeout.tv_sec  = 5;
        select_timeout.tv_usec = 0;


        if (FD_ISSET(from_server, &reader)) {
            int read_ret = Read(from_server, received, MAX_MSG_LEN);

            // If the pipe is closed, handle cleanup and log the event
            if (read_ret == 0) {
                Close(from_server);
                FD_CLR(from_server, &master);
                logging("WARN", "Connection to map process lost. Pipe closed.");
            } else {
                char aux[100];
                
                // If "STOP" command is received, exit the loop  
                if (!strcmp(received, "STOP")) {
                    break;
                }
                switch (received[0]) {
              case 'D':
                        // 'D' indicates a drone position update
                        sscanf(received, "D%f|%f", &drone_pos.x, &drone_pos.y);
                        break;
                    case 'O':
                        // 'O' signals the arrival of new obstacle data
                        logging("INFO", "Processing new obstacle data...");
                        tokenization(obstacles_pos, received, &obstacles_num);
                        sprintf(aux, "Total obstacles updated: %d", obstacles_num);
                        logging("INFO", aux);
                        break;
                    case 'T':
                        // 'T' signals the arrival of new target data
                        logging("INFO", "Processing new target data...");
                        tokenization(targets_pos, received, &target_num);
                        sprintf(aux, "Total targets updated: %d", target_num);
                        logging("INFO", aux);
                        start_time = time(NULL); // Update target spawn time
                        break;
                }
            }
        }

        // Refresh the screen to update the display
        refresh();

        // Display the window title
        mvprintw(0, 0, "MAP DISPLAY");

        // Clear any extra characters when resizing the window
        int max_x = COLS;  
        for (int x = 11; x < max_x; x++) {
            mvaddch(0, x, ' ');
        }

        // Display the current score and event messages
        if (score_increment > 0) {
            attron(COLOR_PAIR(3));  // Green for positive score
            mvprintw(0, COLS / 5, "Score: %d | %s +%d points", score, event_reason, score_increment);
            attroff(COLOR_PAIR(3));
        } else if (score_increment < 0) {
            attron(COLOR_PAIR(2));  // Red for negative score
            mvprintw(0, COLS / 5, "Score: %d | %s -%d points", score, event_reason, -score_increment);
            attroff(COLOR_PAIR(2));  
        } else {
            mvprintw(0, COLS / 5, "Start playing the game!");
        }

        // Refresh to apply the score updates
        refresh();

        // Recreate the map window to handle resizing and animations
        delwin(map_window);  
        map_window = create_map_win(LINES - 1, COLS, 1, 0);


        // Convert the drone's simulated position (500x500 grid) to the terminal window scale.
        // The mapping maintains proportionality between simulation and display dimensions.
        //
        // Adjustments:
        // - The window has borders, so we subtract 2 from width/height.
        // - An extra -1 ensures the correct index range, as array indices start at 0.
        //
        // Formula: 
        // drone_position_in_terminal = (simulated_drone_position * (window_size - border_offset)) / SIMULATION_SIZE

        int drone_x = round(1 + drone_pos.x * (getmaxx(map_window) - 3) / SIMULATION_WIDTH);
        int drone_y = round(1 + drone_pos.y * (getmaxy(map_window) - 3) / SIMULATION_HEIGHT);


        int target_x, target_y;
        bool to_decrease = false;

        // Get the current time
        time_t current_time = time(NULL);
        
        // Last time the score was decreased
        static time_t last_score_decrease_time = 0;

        char to_send[MAX_MSG_LEN];

        // Activate color for displaying targets
        wattron(map_window, COLOR_PAIR(3));

        for (int i = 0; i < target_num; i++) {
            // Convert target's simulated position to fit terminal window dimensions
            target_x = round(1 + targets_pos[i].x * (getmaxx(map_window) - 3) / SIMULATION_WIDTH);
            target_y = round(1 + targets_pos[i].y * (getmaxy(map_window) - 3) / SIMULATION_HEIGHT);

            // Ensure no overlap with other objects
            if (is_overlapping(target_y, target_x, NULL, NULL)) {
                find_spot(&target_y, &target_x, drone_y, drone_x);
            }

            // --- Wall Collision Logic ---
            // If the drone moves outside simulation boundaries, decrease score
            if (drone_pos.y < 1 || drone_pos.y > SIMULATION_HEIGHT - 1 || 
                drone_pos.x < 1 || drone_pos.x > SIMULATION_WIDTH - 1) {
                
                // Only decrease score if 10 seconds have passed since game start
                if (difftime(current_time, start_time) > 10) {
                    
                    // Prevent frequent deductions—only decrease score once every 3 seconds
                    if (difftime(current_time, last_score_decrease_time) > 3) {
                        score_increment = -1;
                        score += score_increment;
                        last_score_decrease_time = current_time;

                        // Update event log message
                        snprintf(event_reason, sizeof(event_reason), "You hit the wall! You lost points.");
                    }
                }
            }
            // Check if the drone has reached the target
            if (target_x == drone_x && target_y == drone_y) {
                // Calculate time taken to reach the target
                time_t impact_time = current_time - last_target_reached_time;
                mvprintw(0, 4 * COLS / 5, "%ld", (long)impact_time);

                // --- Scoring Logic ---
                // If the target 1 is reached within 20 seconds:
                // Score increases based on the formula: 20 - time taken
                // Otherwise, it gives a minimal point increase.
                if (impact_time < 20)// && i == 0) {
                    score_increment = 4 + (20 - (int)ceil(impact_time));
                if (i == 0) {
                    score_increment = 4;     // Target 1 gives 4 points

                } else
                    score_increment = 2;    // Other targets give 2 points.

                // Update score and last target hit time
                score += score_increment;
                last_target_reached_time = time(NULL);

                // Event message on the screen
                snprintf(event_reason, sizeof(event_reason), "You reached target %d! You got", i + 1);
                
                // Notify server of target hit
                sprintf(to_send, "TH|%d|%.3f,%.3f", i, targets_pos[i].x, targets_pos[i].y);
                remove_target(i, targets_pos, target_num);
                Write(to_server, to_send, MAX_MSG_LEN);

                // Mark that a target was removed
                to_decrease = true;
            } else {
                // Store target position for collision checking
                target_obstacles_screen_position[++tosp_top][0] = target_y;
                target_obstacles_screen_position[tosp_top][1]   = target_x;

                // Render the target on the map
                mvwprintw(map_window, target_y, target_x, "%d", i + 1);
            }
        }

        // Disable target color after rendering
        wattroff(map_window, COLOR_PAIR(3));

        // Check if any targets were hit
        if (to_decrease) {
            // If all targets have been hit, request new ones from the server
            if (--target_num == 0) {
                Write(to_server, "GE", MAX_MSG_LEN);
            }
        }

        int obst_x, obst_y;

        // Boolean flag to determine if the drone should be displayed or if it appears 
        // to be overlapping an obstacle. Due to the simulation's 500x500 resolution 
        // being scaled down for the terminal window, visual overlaps may occur. 
        // However, this does not mean the drone is actually colliding with obstacles 
        // in the simulation. 
        //
        // Adjusting obstacle positions to avoid this issue is not ideal, as users 
        // may resize the terminal to a small window, causing the same visual effect.
        bool can_display_drone = true;  

        wattron(map_window, COLOR_PAIR(2));
        for (int i = 0; i < obstacles_num; i++) {
            // Convert obstacle position from simulation space (500x500) to terminal coordinates.
            obst_x = round(1 + obstacles_pos[i].x * (getmaxx(map_window) - 3) / SIMULATION_WIDTH);
            obst_y = round(1 + obstacles_pos[i].y * (getmaxy(map_window) - 3) / SIMULATION_HEIGHT);

        // Check for overlap with existing targets, obstacles, or the drone itself.
        if (is_overlapping(obst_y, obst_x, &drone_y, &drone_x)) {
            find_spot(&obst_y, &obst_x, drone_y, drone_x); // Find an alternative position.
        }

        // Store the obstacle position for future collision checks.
        target_obstacles_screen_position[++tosp_top][0] = obst_y;
        target_obstacles_screen_position[tosp_top][1]   = obst_x;

        // Render the obstacle on the map.
        mvwprintw(map_window, obst_y, obst_x, "O");

        // If the drone's position matches an obstacle, prevent it from being displayed.
        if (obst_y == drone_y && obst_x == drone_x) {
            can_display_drone = false;
        }
        }

        wattroff(map_window, COLOR_PAIR(2)); // Disable obstacle color rendering.

        // Render the drone if it is not visually overlapping an obstacle.
        if (can_display_drone) {
            wattron(map_window, COLOR_PAIR(1));
            mvwprintw(map_window, drone_y, drone_x, "+");
            wattroff(map_window, COLOR_PAIR(1));
        }

        // Refresh the map window to reflect updated positions.
        wrefresh(map_window);
        tosp_top = -1; // Reset the top index for tracking positions.

    }

    /// Clean up
    Close(to_server);
    Close(from_server);
    endwin();
    return EXIT_SUCCESS;
}
