#include "constants.h"
#include "dataStructs.h"
#include "utils/utils.h"
#include "wrapFuncs/wrapFunc.h"
#include <curses.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// WD pid
pid_t WD_pid = -1;

// Create the outer border of the window
WINDOW *input_display_setup(int height, int width, int starty, int startx) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0); /* 0, 0 gives default characters
                           * for the vertical and horizontal
                           * lines			*/
    return local_win;
}

// Destroy the map window. Useful to refresh the window once the terminal is
// resized
void destroy_input_display(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(local_win);
    delwin(local_win);
}

// Function to set the pressed input green
void begin_format_input(int input, WINDOW *tl_win, WINDOW *tc_win,
                        WINDOW *tr_win, WINDOW *cl_win, WINDOW *cc_win,
                        WINDOW *cr_win, WINDOW *bl_win, WINDOW *bc_win,
                        WINDOW *br_win) {

    // In this function the color of the arrows is enabled if the press of
    // the corresponding key in the keyboard is detected
    switch (input) {
        case 'q':
            wattron(tl_win, COLOR_PAIR(1));
            break;
        case 'w':
            wattron(tc_win, COLOR_PAIR(1));
            break;
        case 'e':
            wattron(tr_win, COLOR_PAIR(1));
            break;
        case 'a':
            wattron(cl_win, COLOR_PAIR(1));
            break;
        case 's':
            wattron(cc_win, COLOR_PAIR(1));
            break;
        case 'd':
            wattron(cr_win, COLOR_PAIR(1));
            break;
        case 'z':
            wattron(bl_win, COLOR_PAIR(1));
            break;
        case 'x':
            wattron(bc_win, COLOR_PAIR(1));
            break;
        case 'c':
            wattron(br_win, COLOR_PAIR(1));
            break;
        case ' ':
            wattron(cc_win, COLOR_PAIR(1));
            break;
    }
}

// Function to reset the color of the pressed input
void end_format_input(int input, WINDOW *tl_win, WINDOW *tc_win, WINDOW *tr_win,
                      WINDOW *cl_win, WINDOW *cc_win, WINDOW *cr_win,
                      WINDOW *bl_win, WINDOW *bc_win, WINDOW *br_win) {

    // This function is the reciprocal of the one above infact it ripristinates
    // the color of the previously pressed key
    switch (input) {
        case 'q':
            wattroff(tl_win, COLOR_PAIR(1));
            break;
        case 'w':
            wattroff(tc_win, COLOR_PAIR(1));
            break;
        case 'e':
            wattroff(tr_win, COLOR_PAIR(1));
            break;
        case 'a':
            wattroff(cl_win, COLOR_PAIR(1));
            break;
        case 's':
            wattroff(cc_win, COLOR_PAIR(1));
            break;
        case 'd':
            wattroff(cr_win, COLOR_PAIR(1));
            break;
        case 'z':
            wattroff(bl_win, COLOR_PAIR(1));
            break;
        case 'x':
            wattroff(bc_win, COLOR_PAIR(1));
            break;
        case 'c':
            wattroff(br_win, COLOR_PAIR(1));
            break;
        case ' ':
            wattroff(cc_win, COLOR_PAIR(1));
            break;
    }
}

float diag(float side) {
    // Beeing the sqrt a slow operation and supposing that
    // this system would be required to be the fastest possible
    // a static value for the half of the sqrt of 2 is used instead of the
    // computation of sqrt
    float sqrt2_half = 0.7071;
    // The dimension of the diagonal of the square is returned
    return side * sqrt2_half;
}

// Function to implement the decreasing of the force
float slow_down(void) { return 0.f; }

// Function to calculate the new force after the user has given another input
bool update_force(struct force *to_update, int input, float step,
                  float max_force) {
    // First we need to read the previous value of the force
    // Note that the axis are positioned in this way
    //                     X
    //           +--------->
    //           |
    //           |
    //         Y |
    //           V

    bool ret = true;
    // Depending on the pressed key the force is updated accordingly
    switch (input) {
        case 'q':
            to_update->x_component -= diag(step);
            to_update->y_component -= diag(step);
            break;
        case 'w':
            to_update->y_component -= step;
            break;
        case 'e':
            to_update->x_component += diag(step);
            to_update->y_component -= diag(step);
            break;
        case 'a':
            to_update->x_component -= step;
            break;
        case 's':
            to_update->x_component = slow_down();
            to_update->y_component = slow_down();
            break;
        case 'd':
            to_update->x_component += step;
            break;
        case 'z':
            to_update->x_component -= diag(step);
            to_update->y_component += diag(step);
            break;
        case 'x':
            to_update->y_component += step;
            break;
        case 'c':
            to_update->x_component += diag(step);
            to_update->y_component += diag(step);
            break;
        case ' ':
            to_update->x_component = slow_down();
            to_update->y_component = slow_down();
            break;
        default:
            ret = false;
            break;
    }

    // If the force goes too big than is set as the max value that has
    // been read from the parameters file
    if (to_update->x_component > max_force)
        to_update->x_component = max_force;
    if (to_update->y_component > max_force)
        to_update->y_component = max_force;
    if (to_update->x_component < -max_force)
        to_update->x_component = -max_force;
    if (to_update->y_component < -max_force)
        to_update->y_component = -max_force;
    return ret;
}

int main(int argc, char *argv[]) {

    // Macro to handle the watchdog signals
    HANDLE_WATCHDOG_SIGNALS();

    // Specifying that argc and argv are unused variables
    int to_server_pipe, from_server_pipe;
    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server_pipe);
        sscanf(argv[2], "%d", &from_server_pipe);
    } else {
        printf("Wrong number of arguments in input\n");
        getchar();
        exit(1);
    }

    // Named pipe (fifo) to send the pid to the WD
    int fd;
    Mkfifo(FIFO2_PATH, 0666);

    // Getting the input pid
    int input_pid = getpid();
    char input_pid_str[10];
    sprintf(input_pid_str, "%d", input_pid);

    fd = Open(FIFO2_PATH, O_WRONLY);
    Write(fd, input_pid_str, strlen(input_pid_str) + 1);
    Close(fd);

    // The max value that the force applied to the drone
    // for each axis is read.
    float max_force = get_param("input", "max_force");

    // Initializing data structs
    // The current force as the current velocity at the
    // beginning of the simulation are 0
    struct force drone_current_force       = {0, 0};
    struct velocity drone_current_velocity = {0, 0};

    // The force_step, meaning how much force should be added
    // with each button press is read from the parameters file.
    float force_step             = get_param("input", "force_step");
    struct pos drone_current_pos = {0, 0};

    // Initializing ncurses
    initscr();
    // Disable line buffering
    cbreak();
    // Disable echo of pressed characters in order to not have
    // the keys pressed to make the drone move appear on the screen
    noecho();
    // Hide the cursor in order to net see the carret while it's displaying
    // the interface
    curs_set(0);
    // Enable the colors for ncurses
    start_color();
    // Use the default terminal colors. This is usefull while displaying the
    // current background color and leave it as the same as the one of the
    // current terminal
    use_default_colors();
    // The use of use_default_colors comes into play here where the color of the
    // background is set to -1 meaning that it becomes the current terminal
    // background color
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);

    // The windows of the matrix visible in the left split are now initialized
    WINDOW *left_split  = input_display_setup(LINES, COLS / 2 - 1, 0, 0);
    WINDOW *right_split = input_display_setup(LINES, COLS / 2 - 1, COLS / 2, 0);
    WINDOW *tl_win      = input_display_setup(5, 7, LINES / 3, COLS / 6);
    WINDOW *tc_win      = input_display_setup(5, 7, LINES / 3, COLS / 6 + 6);
    WINDOW *tr_win      = input_display_setup(5, 7, LINES / 3, COLS / 6 + 12);
    WINDOW *cl_win      = input_display_setup(5, 7, LINES / 3 + 4, COLS / 6);
    WINDOW *cc_win = input_display_setup(5, 7, LINES / 3 + 4, COLS / 6 + 6);
    WINDOW *cr_win = input_display_setup(5, 7, LINES / 3 + 4, COLS / 6 + 12);
    WINDOW *bl_win = input_display_setup(5, 7, LINES / 3 + 8, COLS / 6);
    WINDOW *bc_win = input_display_setup(5, 7, LINES / 3 + 8, COLS / 6 + 6);
    WINDOW *br_win = input_display_setup(5, 7, LINES / 3 + 8, COLS / 6 + 12);

    // The variable in which to store the input pressed by the user is
    // initialized here
    char input;

    // The value of the reading params interval for the read from the parameters
    // file is first set here. This is a counter that is decreased for each
    // cycle in the main while loop. It is a way to reduce the number of reads
    // from the parameters file. The parameter is given in seconds. In order to
    // calculate the number of iterations needed to approximately achieve the
    // desired reading rate, the value given needs to be divided by the
    // interval.
    int reading_params_interval =
        round(get_param("input", "reading_params_interval") / 0.1);
    // Also note that reading_params_interval should always be bigger
    // than the interval of the fuction, because reading from file is a slow
    // operation and should be done with low frequency. If the user sets the
    // interval too low and the reading params interval goes under 1 then is
    // set again to 1, because remember that this is a counter in the main loop
    if (reading_params_interval < 1)
        reading_params_interval = 1;

    // Timeout sets the time in milliseconds that getch should wait if no
    // input is received. This is the equivalent of having: usleep(100000)
    // non_blocking_getch()
    timeout(100);

    fd_set reader, master;
    FD_ZERO(&reader);
    FD_ZERO(&master);
    FD_SET(from_server_pipe, &master);

    char aux[MAX_MSG_LEN];

    while (1) {
        // Updating constants at runtime when the reading_params_interval goes
        // to 0
        if (!reading_params_interval--) {
            reading_params_interval =
                get_param("input", "reading_params_interval") / 0.1;
            if (reading_params_interval < 1)
                reading_params_interval = 1;
            force_step = get_param("input", "force_step");
            max_force  = get_param("input", "max_force");
            logging(LOG_INFO, "Input has updated its parameters");
        }

        // Getting user input if present
        input = getch();

        // If the user presses the p key then it's time for all the processes to
        // die in a safe way
        if (input == 'p') {
            // This is not very elegant but the watchdog has to be killed in
            // order to not trigger an emergency shutdown due to the lack of the
            // already killed processes
            if (WD_pid != -1)
                Kill(WD_pid, SIGKILL);

            // Signal the server of the received STOP signal
            Write(to_server_pipe, "STOP", MAX_MSG_LEN);
            break;
        }

        // Calculate the currently acting force on the drone by sending the
        // currently pressed key to the update_force function
        bool to_update =
            update_force(&drone_current_force, input, force_step, max_force);

        // Write to the shared memory the current force value
        // Drone current force x and y
        if (to_update) {
            char aux_force[MAX_STR_LEN];
            sprintf(aux_force, "%f|%f", drone_current_force.x_component,
                    drone_current_force.y_component);
            Write(to_server_pipe, aux_force, MAX_MSG_LEN);
            logging(LOG_INFO, "New input force send to the server");
        }

        // Send update request to server
        Write(to_server_pipe, "U", MAX_MSG_LEN);
        int read_ret = Read(from_server_pipe, aux, MAX_MSG_LEN);
        if (read_ret == 0)
            break;
        sscanf(aux, "%f,%f|%f,%f", &drone_current_pos.x, &drone_current_pos.y,
               &drone_current_velocity.x_component,
               &drone_current_velocity.y_component);

        // Destroy all the windows in order to create the animation and
        // guarantee that the windows will be refreshed in case of the resizing
        // of the terminal
        destroy_input_display(tl_win);
        destroy_input_display(left_split);
        destroy_input_display(right_split);

        // Setting the initial values for the splits
        left_split  = input_display_setup(LINES, COLS / 2 - 1, 0, 0);
        right_split = input_display_setup(LINES, COLS / 2 - 1, 0, COLS / 2);

        // Setting the initial values for the cells of the matrix containing
        // the arrows
        tl_win = input_display_setup(5, 7, LINES / 4, COLS / 6);
        tc_win = input_display_setup(5, 7, LINES / 4, COLS / 6 + 6);
        tr_win = input_display_setup(5, 7, LINES / 4, COLS / 6 + 12);
        cl_win = input_display_setup(5, 7, LINES / 4 + 4, COLS / 6);
        cc_win = input_display_setup(5, 7, LINES / 4 + 4, COLS / 6 + 6);
        cr_win = input_display_setup(5, 7, LINES / 4 + 4, COLS / 6 + 12);
        bl_win = input_display_setup(5, 7, LINES / 4 + 8, COLS / 6);
        bc_win = input_display_setup(5, 7, LINES / 4 + 8, COLS / 6 + 6);
        br_win = input_display_setup(5, 7, LINES / 4 + 8, COLS / 6 + 12);

        // Setting the "titles" of the splits
        mvwprintw(left_split, 0, 1, "INPUT DISPLAY");
        mvwprintw(right_split, 0, 1, "DYNAMICS DISPLAY");

        // Begin the coloring of the pressed key if any key is pressed
        begin_format_input(input, tl_win, tc_win, tr_win, cl_win, cc_win,
                           cr_win, bl_win, bc_win, br_win);

        /// Creating the ascii arts
        // Up-left arrow
        mvwprintw(tl_win, 1, 3, "_");
        mvwprintw(tl_win, 2, 2, "'\\");

        // Up arrow
        mvwprintw(tc_win, 1, 3, "A");
        mvwprintw(tc_win, 2, 3, "|");

        // Up-right arrow
        mvwprintw(tr_win, 1, 3, "_");
        mvwprintw(tr_win, 2, 3, "/'");

        // Left arrow
        mvwprintw(cl_win, 2, 2, "<");
        mvwprintw(cl_win, 2, 3, "-");

        // Right arrow
        mvwprintw(cr_win, 2, 3, "-");
        mvwprintw(cr_win, 2, 4, ">");

        // Down-left arrow
        mvwprintw(bl_win, 2, 2, "|/");
        mvwprintw(bl_win, 3, 2, "'-");

        // Down arrow
        mvwprintw(bc_win, 2, 3, "|");
        mvwprintw(bc_win, 3, 3, "V");

        // Down-right arrow
        mvwprintw(br_win, 2, 3, "\\|");
        mvwprintw(br_win, 3, 3, "-'");

        // Break symbol
        mvwprintw(cc_win, 2, 3, "X");

        // Disable the color for the next iteration
        end_format_input(input, tl_win, tc_win, tr_win, cl_win, cc_win, cr_win,
                         bl_win, bc_win, br_win);

        // Setting the symbols for the corners of the cells
        mvwprintw(tc_win, 0, 0, ".");
        mvwprintw(tr_win, 0, 0, ".");
        mvwprintw(cl_win, 0, 0, "+");
        mvwprintw(cc_win, 0, 0, "+");
        mvwprintw(cr_win, 0, 0, "+");
        mvwprintw(cr_win, 0, 6, "+");
        mvwprintw(bl_win, 0, 0, "+");
        mvwprintw(bc_win, 0, 0, "+");
        mvwprintw(br_win, 0, 0, "+");
        mvwprintw(br_win, 0, 6, "+");
        mvwprintw(bc_win, 4, 0, "'");
        mvwprintw(br_win, 4, 0, "'");

        // Signaling what's the button to close everything
        mvwprintw(left_split, LINES - 3, 3, "Press p to close everything");

        /// Right split

        // Displaying the current position of the drone with nice formatting
        mvwprintw(right_split, LINES / 10 + 2, COLS / 10, "position {");
        mvwprintw(right_split, LINES / 10 + 3, COLS / 10, "\tx: %f",
                  drone_current_pos.x);
        mvwprintw(right_split, LINES / 10 + 4, COLS / 10, "\ty: %f",
                  drone_current_pos.y);
        mvwprintw(right_split, LINES / 10 + 5, COLS / 10, "}");

        // Displaying the current velocity of the drone with nice formatting
        mvwprintw(right_split, LINES / 10 + 7, COLS / 10, "velocity {");
        mvwprintw(right_split, LINES / 10 + 8, COLS / 10, "\tx: %f",
                  drone_current_velocity.x_component);
        mvwprintw(right_split, LINES / 10 + 9, COLS / 10, "\ty: %f",
                  drone_current_velocity.y_component);
        mvwprintw(right_split, LINES / 10 + 10, COLS / 10, "}");

        // Displaying the current force beeing applied on the drone only by the
        // user. So no border effects are taken into consideration while
        // displaying these values.
        mvwprintw(right_split, LINES / 10 + 12, COLS / 10, "force {");
        if (fabs(drone_current_force.x_component) == max_force) {
            wattron(right_split, COLOR_PAIR(2));
            mvwprintw(right_split, LINES / 10 + 13, COLS / 10, "\tx: %f",
                      drone_current_force.x_component);
            wattroff(right_split, COLOR_PAIR(2));
        } else
            mvwprintw(right_split, LINES / 10 + 13, COLS / 10, "\tx: %f",
                      drone_current_force.x_component);
        if (fabs(drone_current_force.y_component) == max_force) {
            wattron(right_split, COLOR_PAIR(2));
            mvwprintw(right_split, LINES / 10 + 14, COLS / 10, "\ty: %f",
                      drone_current_force.y_component);
            wattroff(right_split, COLOR_PAIR(2));
        } else
            mvwprintw(right_split, LINES / 10 + 14, COLS / 10, "\ty: %f",
                      drone_current_force.y_component);

        // Refreshing all the windows
        wrefresh(left_split);
        wrefresh(right_split);
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

    // Cleaning up
    Close(to_server_pipe);
    Close(from_server_pipe);
    // Closing ncurses
    endwin();
    return 0;
}
