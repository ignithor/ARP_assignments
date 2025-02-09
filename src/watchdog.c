#include "constants.h"
#include "utility/utility.h"
#include "wrappers/wrappers.h"
#include <time.h>

// Array to store process PIDs
int p_pids[NUM_PROCESSES];

// PIDs of konsole processes executing Input and Map
int konsole_input_pid	, konsole_map_pid	;

// Counter to track responses from processes to WD
int response_count = 0;

// Check variable for kill status
int kill_status	;

// PID of the detected faulty process
int fault_pid;

/**
 * Signal handler for SIGUSR2 signals received from monitored processes.
 * This indicates that a process has responded to the watchdog.
 */
void watchdog_signal_handler(int signo, siginfo_t *info, void *context) {
    // Marking context and info as unused to avoid compiler warnings
    (void)(info);
    (void)(context);

    // If SIGUSR2 is received, increment the response count
    if (signo == SIGUSR2)
        response_count++;
}


int main(int argc, char *argv[]) {
    // Initialize and configure signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_sigaction = watchdog_signal_handler; // Assign signal handler function
    sigemptyset(&sa.sa_mask);
    
    // SA_RESTART ensures interrupted system calls are restarted
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    // Register signal handler for SIGUSR2
    Sigaction(SIGUSR2, &sa, NULL);

    // Verify the correct number of arguments
    if (argc == NUM_PROCESSES) {
        sscanf(argv[1], "%d", &p_pids[2]);  // Server PID
        sscanf(argv[2], "%d", &p_pids[3]);  // Drone PID
        sscanf(argv[3], "%d", &konsole_input_pid	);  // Konsole running Input
        sscanf(argv[4], "%d", &konsole_map_pid	);  // Konsole running Map
        sscanf(argv[5], "%d", &p_pids[4]);  // Target PID
        sscanf(argv[6], "%d", &p_pids[5]);  // Obstacles PID
    } else {
        perror("Invalid argument list");
        exit(1);
    }

    // Logging message buffer
    char logmsg[100];

    // Retrieve Input process PID through a named pipe (FIFO)
    int fd1;
    Mkfifo(FIFO2_PATH, 0666);

    // Buffer to store the Input process PID
    char input_pid_str[10];

    // Read Input PID from FIFO
    fd1 = Open(FIFO2_PATH, O_RDONLY);
    Read(fd1, input_pid_str, sizeof(input_pid_str));
    Close(fd1);

    // Store the Input PID in the process array
    sscanf(input_pid_str, "%d", &p_pids[0]);

    // Print received Input PID for verification
    printf("Input PID: %s\n", input_pid_str);

    // Retrieve Map process PID and its Konsole PID through another named pipe
    int fd2;
    Mkfifo(FIFO1_PATH, 0666);

    // Buffer to store Map process and Konsole PIDs
    char map_pids_str[20];

    // Read Map process PID from FIFO
    fd2 = Open(FIFO1_PATH, O_RDONLY);
    Read(fd2, map_pids_str, sizeof(map_pids_str));
    Close(fd2);

    // Store Map process PID in the process array
    sscanf(map_pids_str, "%d", &p_pids[1]);

    // Print received Map PID for verification
    printf("Map PID: %d\n", p_pids[1]);


    while (1) {
        // Iterate over all monitored processes (excluding watchdog itself)
        for (int i = 0; i < NUM_PROCESSES - 1; i++) {
            // Send SIGUSR1 signal to the process and store the return value
            kill_status	 = Kill2(p_pids[i], SIGUSR1);

            // Log the signal being sent
            sprintf(logmsg, "WD sending signal to process PID: %d", p_pids[i]);
            logging("INFO", logmsg);

            // Handle interruptions in sleep caused by signals
            // sleep() may return early due to a signal, so we retry until the sleep duration is met
            while (sleep(WD_SLEEP_PERIOD));

            // Check if the process is either dead (kill failed) or frozen (did not increment count)
            if (kill_status	 == -1 || response_count == 0) {
                // Save the PID of the failed process
                fault_pid = p_pids[i];

                // Log the failure and termination of all processes
                sprintf(logmsg, 
                        "WD detected failure in process PID: %d. Terminating all processes.", 
                        fault_pid);
                logging("WARN", logmsg);

                // Kill all monitored processes (excluding Konsole processes)
                for (int i = 0; i < NUM_PROCESSES - 1; i++) {
                    Kill2(p_pids[i], SIGKILL);
                }

                // Kill Konsole processes separately
                Kill2(konsole_input_pid	, SIGKILL);
                Kill2(konsole_map_pid	, SIGKILL);

                // Exit successfully after termination
                return EXIT_SUCCESS;
            }

            // Reset count if the process responded correctly
            response_count = 0;
        }
    }
    return EXIT_SUCCESS;
}