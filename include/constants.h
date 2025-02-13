#ifndef CONSTANTS_H
#define CONSTANTS_H

#define NUM_PROCESSES 7

#define LOGFILE_PATH "../log/process.log"
#define FIFO1_PATH "./fifo_one"
#define FIFO2_PATH "./fifo_two"

#define SIMULATION_WIDTH 400
#define SIMULATION_HEIGHT 400
#define ZERO_THRESHOLD 0.1

#define MAX_STR_LEN 300
#define MAX_MSG_LEN 1024

// Don't change these values without
// changing the corresponding values in the idl files
#define N_TARGETS 9
#define N_OBSTACLES 10

// Maximum combined force from the obstacles
#define MAX_OBST_FORCES 1000
// Maximum combined force from the targets
#define MAX_TARG_FORCES 2000

#define INIT_POSE_X 200
#define INIT_POSE_Y 200

#define OBSTACLES_SPAWN_PERIOD 20

#define TOPIC_NAME_OBSTACLE "obstacle"
#define TOPIC_NAME_TARGET "target"

#define PUBLISHERS_SLEEP_MODE false

// Defining the amount to sleep between any two consequent signals to the
// processes
#define WD_SLEEP_PERIOD 1

#endif // !CONSTANTS_H
