#include "constants.h"
#include "utils/utils.h"
#include "wrapFuncs/wrapFunc.h"
#include <time.h>

// Array for processes' pids
int p_pids[NUM_PROCESSES];

// Pid of the konsole executing input and the map processes
int pid_konsole_input, pid_konsole_map;

// Count to check whether process has replied to WD
int count = 0;

// Check of kill
int check;

// Pid of the dead process
int fault_pid;

// Signal handler for signals received from the monitored processes
void signal_handler_wd(int signo, siginfo_t *info, void *context) {
    // Specifying that context and info are unused
    (void)(info);
    (void)(context);

    // A process responded to SIGUSR1 so increase count
    if (signo == SIGUSR2)
        count++;
}

int main(int argc, char *argv[]) {
    // Signal declaration
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    // Setting the signal handler
    sa.sa_sigaction = signal_handler_wd;
    sigemptyset(&sa.sa_mask);
    // Setting flags
    // The SA_RESTART flag is used to restart all those syscalls that can get
    // interrupted by signals
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    // Enabling the handler with the specified flags
    Sigaction(SIGUSR2, &sa, NULL);

    // argv[1] -> server
    // argv[2] -> drone
    // argv[3] -> konsole of input
    // argv[4] -> konsole of map
    // argv[5] -> target
    // argv[6] -> obstacles
    if (argc == NUM_PROCESSES) {
        sscanf(argv[1], "%d", &p_pids[2]);
        sscanf(argv[2], "%d", &p_pids[3]);
        sscanf(argv[3], "%d", &pid_konsole_input);
        sscanf(argv[4], "%d", &pid_konsole_map);
        sscanf(argv[5], "%d", &p_pids[4]);
        sscanf(argv[6], "%d", &p_pids[5]);
    } else {
        perror("arg_list error");
        exit(1);
    }

    // Auxiliary variable used to create the log message to insert into the
    // logfile
    char logmsg[100];

    // Getting the input pid through the named pipe
    // Initializing the named pipe
    int fd1;
    Mkfifo(FIFO2_PATH, 0666);

    // String to save the input pid
    char input_pid_str[10];

    // Opening the named pipe and reading the input pid from it
    fd1 = Open(FIFO2_PATH, O_RDONLY);
    Read(fd1, input_pid_str, sizeof(input_pid_str));
    Close(fd1);

    // Saving the input pid in the array of processes pids
    sscanf(input_pid_str, "%d", &p_pids[0]);

    // Printing the received input pid to verify its correctness
    printf("Input pid is %s\n", input_pid_str);

    // Getting both the map pid and the pid of the Konsole running map through
    // the named pipe
    // Initializing the named pipe
    int fd2;
    Mkfifo(FIFO1_PATH, 0666);

    // String to save both pids
    char map_pids_str[20];

    // Opening the named pipe and reading the pids from it
    fd2 = Open(FIFO1_PATH, O_RDONLY);
    Read(fd2, map_pids_str, sizeof(map_pids_str));
    Close(fd2);

    // Save map pid in the array of processes pids and the pid of the Konsole
    // running map in a separate int variable.
    // This because the Konsole(s) do not receive nor send signals to the WD,
    // but only need to be killed if a process freezes or dies
    sscanf(map_pids_str, "%d", &p_pids[1]);

    // Printing the received pids to verify their correctness
    printf("Map pid is %d \n", p_pids[1]);

    while (1) {
        // Iterate for the number of processes to check
        for (int i = 0; i < NUM_PROCESSES - 1; i++) {
            // Saving the return value of the kill so that it is possible to
            // immediately verify if the kill failed, so if a process died
            check = Kill2(p_pids[i], SIGUSR1);

            // Writing in the logfile which process the WD sent a signal to
            sprintf(logmsg, "WD sending signal to %d", p_pids[i]);
            logging(LOG_INFO, logmsg);

            // This may seem strange but is required because signals may
            // interrupt sleep. Sleep when interrupted returns the amount of
            // seconds for which it still needed to sleep for. So this is
            // actually restarting the sleep in case it does not return 0. Now
            // it may seem a busy wait but it's actually not because of how
            // signals are handled
            while (sleep(WD_SLEEP_PERIOD))
                ;

            // If either the process is dead, so the kill failed returning -1 to
            // check, or the process is frozen, so it did not increment the
            // count, the WD kills all processes and exits
            if (check == -1 || count == 0) {
                // Saving the pid of the dead/frozen process in a specific
                // variable
                fault_pid = p_pids[i];

                sprintf(logmsg,
                        "WD killed all processes because of process"
                        " with pid %d",
                        fault_pid);
                logging(LOG_WARN, logmsg);

                // Killing all processes, except Konsole's
                for (int i = 0; i < NUM_PROCESSES - 1; i++) {
                    Kill2(p_pids[i], SIGKILL);
                }

                /// Killing Konsole's
                Kill2(pid_konsole_input, SIGKILL);
                Kill2(pid_konsole_map, SIGKILL);

                // Exiting with success
                return EXIT_SUCCESS;
            }
            // if the process is not dead nor frozen, reset its count
            else
                count = 0;
        }
    }
    return EXIT_SUCCESS;
}
