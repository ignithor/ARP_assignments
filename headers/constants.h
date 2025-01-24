#ifndef CONSTANTS_H
#define CONSTANTS_H

#define NUM_PROCESSES 7

#define SHMOBJ_PATH "/shm_server"
#define SEM_PATH_POSITION "/sem_position"
#define SEM_PATH_FORCE "/sem_force"
#define SEM_PATH_VELOCITY "/sem_velocity"

#define LOGFILE_PATH "../log/process.log"
#define FIFO1_PATH "/tmp/fifo_one"
#define FIFO2_PATH "/tmp/fifo_two"

#define MAX_SHM_SIZE 1024
#define SHM_OFFSET_POSITION 0
#define SHM_OFFSET_FORCE_COMPONENTS 100
#define SHM_OFFSET_VELOCITY_COMPONENTS 200

#define SIMULATION_WIDTH 500
#define SIMULATION_HEIGHT 500
#define ZERO_THRESHOLD 0.1

#define LOG_INFO "INFO"
#define LOG_WARN "WARN"
#define LOG_ERROR "ERROR"

#define MAX_STR_LEN 300
#define MAX_MSG_LEN 1024

#define N_TARGETS 9
#define N_OBSTACLES 9
#define MAX_NUMBER 500

// Maximum combined force from the obstacles
#define MAX_OBST_FORCES 1000
// Maximum combined force from the targets
#define MAX_TARG_FORCES 1000

#define OBSTACLES_SPAWN_PERIOD 20

// Defining the amount to sleep between any two consequent signals to the
// processes
#define WD_SLEEP_PERIOD 1

#endif // !CONSTANTS_H
