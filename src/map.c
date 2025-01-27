#include "constants.h"
#include "dataStructs.h"
#include "utils/utils.h"
#include "wrapFuncs/wrapFunc.h"
#include <math.h>
#include <time.h>

// WD pid
pid_t WD_pid;

// This array keeps the position of all the targets and obstacles in order to
// perform collision checking
int target_obstacles_screen_position[N_TARGETS + N_OBSTACLES][2];

// Index for keeping track of the previous array that is handled like a stack
int tosp_top = -1;

// Create the outer border of the window
WINDOW *create_map_win(int height, int width, int starty, int startx) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0); /* 0, 0 gives default characters
                           * for the vertical and horizontal
                           * lines			*/
    return local_win;
}

// Destroy the map window. Useful to refresh the window once the terminal is
// resized
void destroy_map_win(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    /* The parameters taken are
     * 1. win: the window on which to operate
     * 2. ls: character to be used for the left side of the window
     * 3. rs: character to be used for the right side of the window
     * 4. ts: character to be used for the top side of the window
     * 5. bs: character to be used for the bottom side of the window
     * 6. tl: character to be used for the top left corner of the window
     * 7. tr: character to be used for the top right corner of the window
     * 8. bl: character to be used for the bottom left corner of the window
     * 9. br: character to be used for the bottom right corner of the window
     */
    wrefresh(local_win);
    delwin(local_win);
}

bool is_overlapping(int y, int x, int *drone_y, int *drone_x) {
    // This is not strictly overlapping checking but here allows for less code
    // repetition. This checks if the obstacle or target is inside the
    // boundaries If not then it's not a valid position
    if (y < 1 || x < 1 || y > LINES - 3 || x > COLS - 2)
        return true;

    // Checking for overlapping with drone if obstacle
    if (drone_y != NULL && drone_x != NULL)
        if (*drone_y == y && *drone_x == x)
            return true;
    // Checking for overlapping with other targets or obstacles
    for (int i = 0; i <= tosp_top; i++) {
        if (y == target_obstacles_screen_position[i][0] &&
            x == target_obstacles_screen_position[i][1])
            return true;
    }
    return false;
}

void find_spot(int *old_y, int *old_x, int drone_y, int drone_x) {
    // Function to find a valid spot for the obstacles or targets to be
    // positioned It searches arond the current position of the target/obstacle
    // if the desired one it's not available. it keeps searching in squares
    // around the original position. Follows a visual representation where X is
    // the desired position that is not available and ? are the searched places.
    //
    //          ???????
    //    ???   ?     ?
    //    ?X?   ?  X  ?
    //    ???   ?     ?
    //          ???????
    // It keeps searching expanding that square
    int x = *old_x;
    int y = *old_y;

    // Index grows indeterminatelly and determines the size of the square
    for (int index = 1;; index++) {
        // Check if there is a free spot on the row over the current position
        y = *old_y - index;
        for (int x = *old_x - index; x <= *old_x + index; x++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // Check if there is a free spot on the row below the current position
        y = *old_y + index;
        for (int x = *old_x - index; x <= *old_x + index; x++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // Check if there is a free spot on the column before the current
        // position
        x = *old_x - index;
        for (int y = *old_y - index + 1; y <= *old_y + index - 1; y++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // Check if there is a free spot on the column after the current
        // position
        x = *old_x + index;
        for (int y = *old_y - index + 1; y <= *old_y + index - 1; y++) {
            if (!is_overlapping(y, x, &drone_y, &drone_x)) {
                *old_y = y;
                *old_x = x;
                return;
            }
        }

        // If the index is growing too much than it means that there is no more
        // a free position available
        if (index > 100) {
            logging(LOG_ERROR,
                    "Unable to find a spot on the screen to print on map");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    // Macro to handle the watchdog signals
    HANDLE_WATCHDOG_SIGNALS();

    int to_server, from_server;
    if (argc == 3) {
        sscanf(argv[1], "%d", &to_server);
        sscanf(argv[2], "%d", &from_server);
    } else {
        printf("Wrong number of arguments in map\n");
        getchar();
        exit(1);
    }

    // score variables
    int score           = 0;
    int score_increment = 0;
    // time when the target are spawned
    time_t start_time;

    // Named pipe (fifo) to send the pid to the WD
    Mkfifo(FIFO1_PATH, 0666);

    // Getting the map pid
    int map_pid = getpid();
    char map_pid_str[10];

    // Here the pid of the map process is passed to the WD
    sprintf(map_pid_str, "%d", map_pid);

    // Writing to the fifo the previously formatted string
    int fd;
    fd = Open(FIFO1_PATH, O_WRONLY);
    Write(fd, map_pid_str, strlen(map_pid_str) + 1);
    Close(fd);

    // Setting up the struct in which to store the position of the drone
    // in order to calculate the current position on the screen of the drone
    struct pos drone_pos = {0, 0};

    // Arrays managed as stacks where to save the position of targets and
    // obstacles
    struct pos targets_pos[N_TARGETS];
    int target_num = 0;
    struct pos obstacles_pos[N_OBSTACLES];
    int obstacles_num = 0;

    // Setting up ncurses
    initscr();
    // Disabling line buffering
    cbreak();
    // Hide the cursor in order to not see the white carret on the screen
    curs_set(0);
    // Display the drone, obstacles and targets in color
    start_color();
    use_default_colors();
    // The use of use_default_colors comes into play here where the color of the
    // background is set to -1 meaning that it becomes the current terminal
    // background color
    init_pair(1, COLOR_BLUE, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_GREEN, -1);
    // Displaying the first intance of the window

    WINDOW *map_window =
        create_map_win(getmaxy(stdscr) - 2, getmaxx(stdscr), 1, 0);

    // Array where to save received strings
    char received[MAX_MSG_LEN];

    // Setting up structs for select
    struct timeval select_timeout;
    // Setting a sleep time of 5 seconds in case no update is needed
    select_timeout.tv_sec  = 5;
    select_timeout.tv_usec = 0;
    fd_set reader, master;
    FD_ZERO(&reader);
    FD_ZERO(&master);
    FD_SET(from_server, &master);
    while (1) {
        // resetting the fd_set
        reader = master;
        int ret;
        do {
            // Note that no signal is ignored here because the signal that would
            // be sent to this process is SIG_WINCH that is also a signal that
            // has its own handler defined in ncurses. If here we would have set
            // sig_ign of sig_winch and then after the select restored with
            // sig_dfl then the resizing of the window would not have worked
            // anymore and the gui would have kept its original size
            ret = Select(from_server + 1, &reader, NULL, NULL, &select_timeout);

            // The only reason to get an erorr is if Select gets interrupted by
            // a signal. In that case the function should be restarted if the
            // SA_RESTART flag didn't do its job
        } while (ret == -1);
        // Resetting the timeout
        select_timeout.tv_sec  = 5;
        select_timeout.tv_usec = 0;

        if (FD_ISSET(from_server, &reader)) {
            int read_ret = Read(from_server, received, MAX_MSG_LEN);
            // Check if any pipe needs to be closed
            if (read_ret == 0) {
                Close(from_server);
                FD_CLR(from_server, &master);
                logging(LOG_WARN, "Pipe to map closed");
            } else {
                char aux[100];
                // If STOP then this needs to be closed
                if (!strcmp(received, "STOP")) {
                    break;
                }
                switch (received[0]) {
                    case 'D':
                        // D indicates the drone position
                        sscanf(received, "D%f|%f", &drone_pos.x, &drone_pos.y);
                        break;
                    case 'O':
                        // O indicates that new obstacles are available
                        logging(LOG_INFO,
                                "vvvvvvvvvvvOBSTACLESvvvvvvvvvvvvvvv");
                        tokenization(obstacles_pos, received, &obstacles_num);
                        logging(LOG_INFO,
                                "^^^^^^^^^^^OBSTACLES^^^^^^^^^^^^^^^");
                        sprintf(aux, "Obtacles %d", obstacles_num);
                        logging(LOG_INFO, aux);
                        break;
                    case 'T':
                        // T means that new targets are available
                        logging(LOG_INFO, "vvvvvvvvvvvTARGETSvvvvvvvvvvvvvvv");
                        tokenization(targets_pos, received, &target_num);
                        logging(LOG_INFO, "^^^^^^^^^^^TARGETS^^^^^^^^^^^^^^^");
                        sprintf(aux, "Targets %d", target_num);
                        logging(LOG_INFO, aux);
                        start_time = time(NULL);
                        break;
                }
            }
        }

        // Display the menu text
        refresh();

        mvprintw(0, COLS / 3, "                            ");

        // Displaying the title of the window.
        mvprintw(0, 0, "MAP DISPLAY");

        if (score > 0) {
            attron(COLOR_PAIR(3));
            mvprintw(0, COLS / 3, "Score: %d", score);
            attroff(COLOR_PAIR(3));
        } else if (score < 0) {
            attron(COLOR_PAIR(2));
            mvprintw(0, COLS / 3, "Score: %d", score);
            attroff(COLOR_PAIR(2));
        } else
            mvprintw(0, COLS / 3, "Score: %d", score);
        refresh();

        // Deleting the old window that is encapsulating the map in order to
        // create the animation, and to allow the resizing of the window in case
        // of terminal resize
        delwin(map_window);
        // Redrawing the window. This is useful if the screen is resized
        map_window = create_map_win(LINES - 1, COLS, 1, 0);

        // In order to correctly handle the drone position calculation all
        // the simulations are done in a 500x500 square. Then the position of
        // the drone is converted to the dimension of the window by doing a
        // proportion. The computation done in the following line could by
        // expressed as the following in the x axis:
        // drone_position_in_terminal = simulated_drone_pos_x * term_width/500
        // Now for the terminal width and height it needs to be taken into
        // consideration the border of the window itself. Considering for
        // example the x axis we have a border on the left and one on the right
        // so we need to subtract 2 from the width of the main_window. Now the
        // extra -1 is due to the fact that the index starts from 0 and not
        // from 1. With this we mean that if we have an array of dimension 3.
        // The highest index of an element in the arry will be 2, not 3. This
        // explains why -3 instead of -2.
        int drone_x = round(1 + drone_pos.x * (getmaxx(map_window) - 3) /
                                    SIMULATION_WIDTH);
        int drone_y = round(1 + drone_pos.y * (getmaxy(map_window) - 3) /
                                    SIMULATION_HEIGHT);

        int target_x, target_y;
        bool to_decrease = false;
        // Get the current time
        time_t current_time = time(NULL);
        // Record the last time the score was decreased
        static time_t last_score_decrease_time = 0;
        char to_send[MAX_MSG_LEN];
        // Activate color for the targets
        wattron(map_window, COLOR_PAIR(3));
        for (int i = 0; i < target_num; i++) {
            // Here the targets are displayed in the window by resizing from the
            // simulation window.
            target_x = round(1 + targets_pos[i].x * (getmaxx(map_window) - 3) /
                                     SIMULATION_WIDTH);
            target_y = round(1 + targets_pos[i].y * (getmaxy(map_window) - 3) /
                                     SIMULATION_HEIGHT);
            // It's verified that no target overlaps with any of the previously
            // set
            if (is_overlapping(target_y, target_x, NULL, NULL))
                find_spot(&target_y, &target_x, drone_y, drone_x);
            // if we touch the wall, the score decreases by 1.
            // The score will not decrease for 3 seconds after hitting a wall
            // once.
            if (drone_y == 2 || drone_y == LINES - 3 || drone_x == 1 ||
                drone_x == COLS - 2) {
                if (difftime(current_time, start_time) > 10) {
                    if (difftime(current_time, last_score_decrease_time) > 3) {
                        score -= 1;
                        last_score_decrease_time = current_time;
                    }
                }
            }
            if (target_x == drone_x && target_y == drone_y) {
                // time_t impact_time = time(NULL) - start_time;
                time_t impact_time = current_time - start_time;
                // If a target is reached in the first 20 seconds, the score
                // increases of 20 - the number of seconds taken to reach it.
                // For example, if a target is reached in 5 or more seconds, but
                // less than 6, the score increases by 20-5=15. In general, the
                // score increases of:
                //- 20 - (integer part of impact_time), if impact_time < 20
                //- 1, if impact_time >= 20
                if (impact_time < 20.0)
                    score_increment = 20 - (int)ceil(impact_time);
                // If we reached the target 1, the score increases by 2.
                if (i == 0) {
                    score_increment = 2;
                } else
                    score_increment = 1;
                score = score + score_increment;

                // If the target position is the same as the drone then we have
                // a hit
                sprintf(to_send, "TH|%d|%.3f,%.3f", i, targets_pos[i].x,
                        targets_pos[i].y);
                // The target is then removed from the array
                remove_target(i, targets_pos, target_num);
                Write(to_server, to_send, MAX_MSG_LEN);
                to_decrease = true;
            } else {
                // If no target has been hit then add it to this array that aims
                // to simplify the checking for callisions
                target_obstacles_screen_position[++tosp_top][0] = target_y;
                target_obstacles_screen_position[tosp_top][1]   = target_x;
                // And print the target on screen
                mvwprintw(map_window, target_y, target_x, "%d", i + 1);
            }
        }
        wattroff(map_window, COLOR_PAIR(3));
        // Check whether all the targets have been hit
        if (to_decrease) {
            if (!--target_num)
                // And in that case Write to the server that we need new targets
                Write(to_server, "GE", MAX_MSG_LEN);
        }

        int obst_x, obst_y;
        // Boolean to indicate if the drone can be displayed or is corrently
        // under an obstacle. Note that since the simulation window is 500x500
        // the display on the terminal is resized and much resolution is lost.
        // Because of this ever if in the terminal the two seem overlapping this
        // DOES NOT mean that are really overlapping. It's just a visual thing.
        // We decided that changing values to avoid this issue is not worth it
        // because someone could use the terminal in a very small window and
        // still have this problem.
        bool can_display_drone = true;
        wattron(map_window, COLOR_PAIR(2));
        for (int i = 0; i < obstacles_num; i++) {
            obst_x = round(1 + obstacles_pos[i].x * (getmaxx(map_window) - 3) /
                                   SIMULATION_WIDTH);
            obst_y = round(1 + obstacles_pos[i].y * (getmaxy(map_window) - 3) /
                                   SIMULATION_HEIGHT);
            // Checking for overlapping with previous targets, obstecles and in
            // this case also with the drone
            if (is_overlapping(obst_y, obst_x, &drone_y, &drone_x))
                find_spot(&obst_y, &obst_x, drone_y, drone_x);
            target_obstacles_screen_position[++tosp_top][0] = obst_y;
            target_obstacles_screen_position[tosp_top][1]   = obst_x;
            mvwprintw(map_window, obst_y, obst_x, "O");

            // If the drone is overlapped to an obstacle than it cannot be
            // displayed
            if (obst_y == drone_y && obst_x == drone_x)
                can_display_drone = false;
        }
        wattroff(map_window, COLOR_PAIR(2));

        // The drone is now displayed on the screen
        if (can_display_drone) {
            wattron(map_window, COLOR_PAIR(1));
            mvwprintw(map_window, drone_y, drone_x, "+");
            wattroff(map_window, COLOR_PAIR(1));
        }
        // The map_window is refreshed
        wrefresh(map_window);
        tosp_top = -1;
    }

    /// Clean up
    Close(to_server);
    Close(from_server);
    endwin();
    return EXIT_SUCCESS;
}
