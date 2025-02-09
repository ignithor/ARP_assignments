#include "constants.h"
#include "droneDataStructs.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"
#include <math.h>

// Creates a window with a border
// Parameters: height, width, starty (y position), startx (x position)
WINDOW *input_display_setup(int height, int width, int starty, int startx) {
    WINDOW *local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0); // Draw default border
    return local_win;
}

// Destroys a window and clears its border.
// Useful for refreshing or resizing the display.
void destroy_input_display(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); 
    wrefresh(local_win);  
    delwin(local_win);    
}

// Highlights the corresponding window in green when the respective key is pressed.
void begin_format_input(int input, WINDOW *tl_win, WINDOW *tc_win,
                        WINDOW *tr_win, WINDOW *cl_win, WINDOW *cc_win,
                        WINDOW *cr_win, WINDOW *bl_win, WINDOW *bc_win,
                        WINDOW *br_win) {
    switch (input) {
        case 'q': wattron(tl_win, COLOR_PAIR(1)); break; // Top-left
        case 'w': wattron(tc_win, COLOR_PAIR(1)); break; // Top-center
        case 'e': wattron(tr_win, COLOR_PAIR(1)); break; // Top-right
        case 'a': wattron(cl_win, COLOR_PAIR(1)); break; // Center-left
        case 's': wattron(cc_win, COLOR_PAIR(1)); break; // Center
        case 'd': wattron(cr_win, COLOR_PAIR(1)); break; // Center-right
        case 'z': wattron(bl_win, COLOR_PAIR(1)); break; // Bottom-left
        case 'x': wattron(bc_win, COLOR_PAIR(1)); break; // Bottom-center
        case 'c': wattron(br_win, COLOR_PAIR(1)); break; // Bottom-right
        case ' ': wattron(cc_win, COLOR_PAIR(1)); break; // Space (Center)
    }
}

// Resets the color of the window corresponding to the previously pressed key.
void end_format_input(int input, WINDOW *tl_win, WINDOW *tc_win, WINDOW *tr_win,
                      WINDOW *cl_win, WINDOW *cc_win, WINDOW *cr_win,
                      WINDOW *bl_win, WINDOW *bc_win, WINDOW *br_win) {
    switch (input) {
        case 'q': wattroff(tl_win, COLOR_PAIR(1)); break; // Top-left
        case 'w': wattroff(tc_win, COLOR_PAIR(1)); break; // Top-center
        case 'e': wattroff(tr_win, COLOR_PAIR(1)); break; // Top-right
        case 'a': wattroff(cl_win, COLOR_PAIR(1)); break; // Center-left
        case 's': wattroff(cc_win, COLOR_PAIR(1)); break; // Center
        case 'd': wattroff(cr_win, COLOR_PAIR(1)); break; // Center-right
        case 'z': wattroff(bl_win, COLOR_PAIR(1)); break; // Bottom-left
        case 'x': wattroff(bc_win, COLOR_PAIR(1)); break; // Bottom-center
        case 'c': wattroff(br_win, COLOR_PAIR(1)); break; // Bottom-right
        case ' ': wattroff(cc_win, COLOR_PAIR(1)); break; // Space (Center)
    }
}

// Calculates the diagonal length of a square given its side length.
// Uses a precomputed value for sqrt(2)/2 (0.7071) to optimize performance.
float diag(float side) {
    const float sqrt2_half = 0.7071; // Precomputed value for sqrt(2)/2
    return side * sqrt2_half;
}

// Reduces the force to zero, effectively stopping movement.
float slow_down(void) {
    return 0.0f;
}

// Updates the force applied to the drone based on user input.
// Returns true if the input is recognized, false otherwise.
bool update_force(struct force *to_update, int input, float step, float max_force) {
    /*
     * Coordinate system:
     *                     X
     *           +--------->
     *           |
     *           |
     *         Y |
     *           V
     */
    bool ret = true;

    // Adjust force based on the input key
    switch (input) {
        case 'q': // Top-left diagonal
            to_update->x_component -= diag(step);
            to_update->y_component -= diag(step);
            break;
        case 'w': // Up
            to_update->y_component -= step;
            break;
        case 'e': // Top-right diagonal
            to_update->x_component += diag(step);
            to_update->y_component -= diag(step);
            break;
        case 'a': // Left
            to_update->x_component -= step;
            break;
        case 's': // Stop (reset force)
        case ' ':
            to_update->x_component = slow_down();
            to_update->y_component = slow_down();
            break;
        case 'd': // Right
            to_update->x_component += step;
            break;
        case 'z': // Bottom-left diagonal
            to_update->x_component -= diag(step);
            to_update->y_component += diag(step);
            break;
        case 'x': // Down
            to_update->y_component += step;
            break;
        case 'c': // Bottom-right diagonal
            to_update->x_component += diag(step);
            to_update->y_component += diag(step);
            break;
        default: // Invalid input
            ret = false;
            break;
    }

    // Clamp force components within the maximum limits
    if (to_update->x_component > max_force) to_update->x_component = max_force;
    if (to_update->y_component > max_force) to_update->y_component = max_force;
    if (to_update->x_component < -max_force) to_update->x_component = -max_force;
    if (to_update->y_component < -max_force) to_update->y_component = -max_force;

    return ret;
}

int main(int argc, char *argv[]) {

    // Initialize the watchdog signal handler
    HANDLE_WATCHDOG_SIGNALS();

    // Validate and parse input arguments
    int server_write_pipe, server_read_pipe;
    if (argc == 3) {
        sscanf(argv[1], "%d", &server_write_pipe);   // Extract the "to server" pipe
        sscanf(argv[2], "%d", &server_read_pipe); // Extract the "from server" pipe
    } else {
        printf("Error: Incorrect number of arguments provided.\n");
        getchar();
        exit(1);
    }

    // Create a named pipe (FIFO) to communicate the PID to the watchdog process
    int fd;
    Mkfifo(FIFO2_PATH, 0666);

    // Get the current process ID (PID)
    int input_pid = getpid();
    char input_pid_str[10];
    sprintf(input_pid_str, "%d", input_pid);

    // Write the PID to the watchdog FIFO
    fd = Open(FIFO2_PATH, O_WRONLY);
    Write(fd, input_pid_str, strlen(input_pid_str) + 1);
    Close(fd);


    // Retrieve configuration values
    float max_force = get_param("input", "max_force");   // Max force applied per axis
    float force_step = get_param("input", "force_step"); // Force increment per key press

    // Initialize drone parameters (force, velocity, position)
    struct force drone_force = {0, 0};
    struct velocity drone_velocity = {0, 0};
    struct pos drone_position = {0, 0};

    // Initialize ncurses for UI rendering
    initscr();
    cbreak();      // Disable line buffering for instant input
    noecho();      // Prevent typed characters from appearing on the screen
    curs_set(0);   // Hide cursor for better UI experience
    start_color(); // Enable color support

    // Use terminal's default background color
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1); // Green for active input
    init_pair(2, COLOR_RED, -1);   // Red for warnings/errors


    // The windows of the matrix visible in the left split are now initialized
    WINDOW *control_window  = input_display_setup(LINES, COLS / 2 - 1, 0, 0);
    WINDOW *info_window = input_display_setup(LINES, COLS / 2 - 1, COLS / 2, 0);
    WINDOW *tl_win      = input_display_setup(5, 7, LINES / 3, COLS / 6);
    WINDOW *tc_win      = input_display_setup(5, 7, LINES / 3, COLS / 6 + 6);
    WINDOW *tr_win      = input_display_setup(5, 7, LINES / 3, COLS / 6 + 12);
    WINDOW *cl_win      = input_display_setup(5, 7, LINES / 3 + 4, COLS / 6);
    WINDOW *cc_win = input_display_setup(5, 7, LINES / 3 + 4, COLS / 6 + 6);
    WINDOW *cr_win = input_display_setup(5, 7, LINES / 3 + 4, COLS / 6 + 12);
    WINDOW *bl_win = input_display_setup(5, 7, LINES / 3 + 8, COLS / 6);
    WINDOW *bc_win = input_display_setup(5, 7, LINES / 3 + 8, COLS / 6 + 6);
    WINDOW *br_win = input_display_setup(5, 7, LINES / 3 + 8, COLS / 6 + 12);

    // Variable to store user input
    char input;

    // Set the interval for reading parameters from the file (converted to loop cycles)
    int reading_params_interval = round(get_param("input", "reading_params_interval") / 0.1);

    // Ensure the interval is at least 1 to avoid excessive file reads
    if (reading_params_interval < 1)
        reading_params_interval = 1;

    // Set timeout for non-blocking input (100ms equivalent to usleep(100000))
    timeout(100);

    // Initialize file descriptor sets for monitoring input from the server
    fd_set reader, master;
    FD_ZERO(&reader);
    FD_ZERO(&master);
    FD_SET(server_read_pipe, &master);

    // Buffer for message storage
    char server_response[MAX_MSG_LEN];

    while (1) {
        // Update parameters when the counter reaches zero
        if (!reading_params_interval--) {
            reading_params_interval = round(get_param("input", "reading_params_interval") / 0.1);
            if (reading_params_interval < 1) 
                reading_params_interval = 1;

            force_step = get_param("input", "force_step");
            max_force  = get_param("input", "max_force");

            logging("INFO", "Updated input parameters at runtime.");
        }

        // Capture user input if available
        input = getch();

        // If 'p' is pressed, signal termination to the server and exit
        if (input == 'p') {
            Write(server_write_pipe, "STOP", MAX_MSG_LEN);
            break;
        }

        // Compute the drone's force based on user input
        bool to_update = update_force(&drone_force, input, force_step, max_force);

        // If the force was updated, send the new force values to the server
        if (to_update) {
            char force_message[MAX_STR_LEN];
            sprintf(force_message, "%f|%f", drone_force.x_component, drone_force.y_component);
            Write(server_write_pipe, force_message, MAX_MSG_LEN);
            logging("INFO", "Sent updated input force to the server");
        }

        // Request an update from the server
        Write(server_write_pipe, "U", MAX_MSG_LEN);

        // Read the updated position and velocity from the server
        if (Read(server_read_pipe, server_response, MAX_MSG_LEN) == 0)
            break;
        sscanf(server_response, "%f,%f|%f,%f", &drone_position.x, &drone_position.y,
               &drone_velocity.x_component, &drone_velocity.y_component);

        // Refresh display by destroying and recreating windows
        destroy_input_display(tl_win);
        destroy_input_display(control_window);
        destroy_input_display(info_window);

        // Initialize left and right display splits
        control_window  = input_display_setup(LINES, COLS / 2 - 1, 0, 0);
        info_window = input_display_setup(LINES, COLS / 2 - 1, 0, COLS / 2);

        // Initialize arrow key display windows in a 3x3 grid
        tl_win = input_display_setup(5, 7, LINES / 4, COLS / 6);
        tc_win = input_display_setup(5, 7, LINES / 4, COLS / 6 + 6);
        tr_win = input_display_setup(5, 7, LINES / 4, COLS / 6 + 12);
        cl_win = input_display_setup(5, 7, LINES / 4 + 4, COLS / 6);
        cc_win = input_display_setup(5, 7, LINES / 4 + 4, COLS / 6 + 6);
        cr_win = input_display_setup(5, 7, LINES / 4 + 4, COLS / 6 + 12);
        bl_win = input_display_setup(5, 7, LINES / 4 + 8, COLS / 6);
        bc_win = input_display_setup(5, 7, LINES / 4 + 8, COLS / 6 + 6);
        br_win = input_display_setup(5, 7, LINES / 4 + 8, COLS / 6 + 12);

        // Set titles for display sections
        mvwprintw(control_window, 0, 1, "INPUT DISPLAY");
        mvwprintw(info_window, 0, 1, "DYNAMICS DISPLAY");

        // Highlight the pressed key
        begin_format_input(input, tl_win, tc_win, tr_win, cl_win, cc_win,
                           cr_win, bl_win, bc_win, br_win);

        /// Drawing ASCII arrows
        // Upward arrows
        mvwprintw(tl_win, 1, 3, "_");  // Top-left
        mvwprintw(tl_win, 2, 2, "'\\");
        mvwprintw(tc_win, 1, 3, "A");  // Top-center
        mvwprintw(tc_win, 2, 3, "|");
        mvwprintw(tr_win, 1, 3, "_");  // Top-right
        mvwprintw(tr_win, 2, 3, "/'");

        // Left and right arrows
        mvwprintw(cl_win, 2, 2, "<");  // Left
        mvwprintw(cl_win, 2, 3, "-");
        mvwprintw(cr_win, 2, 3, "-");  // Right
        mvwprintw(cr_win, 2, 4, ">");

        // Downward arrows
        mvwprintw(bl_win, 2, 2, "|/"); // Bottom-left
        mvwprintw(bl_win, 3, 2, "'-");
        mvwprintw(bc_win, 2, 3, "|");  // Bottom-center
        mvwprintw(bc_win, 3, 3, "V");
        mvwprintw(br_win, 2, 3, "\\|"); // Bottom-right
        mvwprintw(br_win, 3, 3, "-'");

        // Stop symbol (brake)
        mvwprintw(cc_win, 2, 3, "X");

        // Reset color for the next iteration
        end_format_input(input, tl_win, tc_win, tr_win, cl_win, cc_win, cr_win,
                         bl_win, bc_win, br_win);

        /// Setting corner symbols in the matrix
        mvwprintw(tc_win, 0, 0, ".");  // Top border corners
        mvwprintw(tr_win, 0, 0, ".");
        mvwprintw(cl_win, 0, 0, "+");  // Middle section corners
        mvwprintw(cc_win, 0, 0, "+");
        mvwprintw(cr_win, 0, 0, "+");
        mvwprintw(cr_win, 0, 6, "+");
        mvwprintw(bl_win, 0, 0, "+");  // Bottom border corners
        mvwprintw(bc_win, 0, 0, "+");
        mvwprintw(br_win, 0, 0, "+");
        mvwprintw(br_win, 0, 6, "+");
        mvwprintw(bc_win, 4, 0, "'");  // Bottom edge symbols
        mvwprintw(br_win, 4, 0, "'");

        // Display instruction to exit
        mvwprintw(control_window, LINES - 3, 3, "Press 'p' to exit");

        /// Right Panel - Display Drone Information

        // Display drone position
        mvwprintw(info_window, LINES / 10 + 2, COLS / 10, "Position {");
        mvwprintw(info_window, LINES / 10 + 3, COLS / 10, "\tx: %f", drone_position.x);
        mvwprintw(info_window, LINES / 10 + 4, COLS / 10, "\ty: %f", drone_position.y);
        mvwprintw(info_window, LINES / 10 + 5, COLS / 10, "}");

        // Display drone velocity
        mvwprintw(info_window, LINES / 10 + 7, COLS / 10, "Velocity {");
        mvwprintw(info_window, LINES / 10 + 8, COLS / 10, "\tx: %f", drone_velocity.x_component);
        mvwprintw(info_window, LINES / 10 + 9, COLS / 10, "\ty: %f", drone_velocity.y_component);
        mvwprintw(info_window, LINES / 10 + 10, COLS / 10, "}");

        // Display the force currently applied by the user on the drone.
        // External effects (e.g., borders) are not considered in these values.
        mvwprintw(info_window, LINES / 10 + 12, COLS / 10, "Force {");

        // Highlight force values in red if they reach the maximum limit
        if (fabs(drone_force.x_component) == max_force) {
            wattron(info_window, COLOR_PAIR(2));
        }
        mvwprintw(info_window, LINES / 10 + 13, COLS / 10, "\tx: %f", drone_force.x_component);
        wattroff(info_window, COLOR_PAIR(2));

        if (fabs(drone_force.y_component) == max_force) {
            wattron(info_window, COLOR_PAIR(2));
        }
        mvwprintw(info_window, LINES / 10 + 14, COLS / 10, "\ty: %f", drone_force.y_component);
        wattroff(info_window, COLOR_PAIR(2));

        // Refresh all windows to update the display
        wrefresh(control_window);
        wrefresh(info_window);
        wrefresh(tl_win);
        wrefresh(tc_win);
        wrefresh(tr_win);
        wrefresh(cl_win);
        wrefresh(cc_win);
        wrefresh(cr_win);
        wrefresh(bl_win);
        wrefresh(bc_win);
        wrefresh(br_win);
    }

    // Cleanup and exit
    Close(server_write_pipe);
    Close(server_read_pipe);
    endwin(); // Close ncurses
    return 0;
}