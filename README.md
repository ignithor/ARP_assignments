# ARP_assignments

# Authors

Yui MOMIYAMA
Paul PHAM DANG
Group 3

## How to run

### Dependencies

These dependencies are needed:

- make
- CMake version > 3.6
- libncurses
    sudo apt install libncurses5-dev libncursesw5-dev
- lib cjson
    sudo apt install libcjson-dev

The assignment has been tried using Ubuntu 22.04

### Run the assignments

Simply execute the scripts by executing:

For the assignment 1 :

    ./run_assignment1.sh

For assignment 2 :

    ./run_assignment2.sh

To change from assignment 1 and 2, you can do 
    git checkout assignment

## Rules of the game
The score is updated based on the following conditions.

### Score Increment Rules
- **If \( t <= 30 \):**
  - If the target number is 1:  
    `score_increment = 4 + (30 - t)`
  - If the target number is not 1:  
    `score_increment = 2`
- **If \( t > 30 \):**
  - If the target number is 1:  
    `score_increment = 4`
  - If the target number is not 1:  
    `score_increment = 2`

### Penalty Rules
- **If the player hits a wall:**  
  The score is decreased by 1:  
  `score_decrement = 1`

Summary table of the scoring rules is shown below.

| Condition        | Target Number | Score Change         |
|------------------|---------------|----------------------|
|  t < 30      | 1             | 4 + (30 - t)   |
|  t < 30      | Not 1         | +2       |
|  t >= 30  | 1             | +4              |
|  t >= 30  | Not 1         | +2              |
| Hit a wall       | -             | -1              |

This ensures that:
- Hitting a new target quickly rewards the player more when the target number is 1.
- All other target hits follow a fixed increment.
- Hitting a wall results in a penalty, reducing the score by 1.

## Technical notes

### Software architecture of the first assignment

![plot](./docs/architecture.jpg)

### Active components
The active components of this project are:

- Server
- Map
- Drone
- Input
- Watchdog
- Target
- Obstacle

They are all launch by the master process

#### Server

The server manages a blackboard with the geometrical state of the world (map, drone, targets, obstacles…). The server read from the pipes coming from the processes and send to the data to another process. Moreover, it also "fork" the **map** process.

#### Map

The **map** process display using ncurses the drone, targets and obstacles. All the data are coming through the pipe from the server. This process also compute the score of the user.

#### Drone

The code processes incoming messages to update obstacle data or drone force components, then calculates and resets the total repulsive forces from obstacles. It iterates through each obstacle to compute the repulsive force based on the distance from the drone, applying the force if within a specified range.

#### Input

The update_force function adjusts the x and y components of a force vector based on user input, clamping the values within specified maximum limits. It handles various directional inputs, including diagonal movements and stopping, and returns a boolean indicating whether the input was valid.

#### Watchdog
The code sets up a signal handler for `SIGUSR2` to increment `response_count` when the signal is received. In the `main` function, it initializes the signal handler, verifies the correct number of command-line arguments, and parses PIDs for various processes, storing them in appropriate variables.

#### Target
The code initializes a target generation process, validates input arguments, and communicates with a server by sending randomly generated target positions. It continuously generates and sends target data until a "STOP" signal is received, then performs cleanup and exits.


#### Obstacle
The code initializes a process to generate and send random obstacle positions to a server, using pipes for communication. It continuously generates obstacle data, sends it to the server, and handles server responses until a "STOP" signal is received, then performs cleanup and exits.

#### Master
The code initializes the master process, creates a log file, and logs the start of the process. It defines an array to store the names of processes to be spawned and includes a function to execute commands for spawning new processes.

### Other files
The other main files of this project are:

- wrappers
- utility
- constant
- droneDataStructs
- drone_parameters.json

#### wrappers
The wrappers.c file provides custom wrapper functions for system calls, enhancing them with detailed error handling and logging. These functions ensure robust error reporting and graceful program termination in case of failures.

#### utility
The utility.c file provides various utility functions, such as reading configuration parameters from a JSON file and tokenizing strings. These functions support the main program by handling common tasks and simplifying code reuse.

#### constant
The constants.h file defines essential constants and macros used throughout the project, such as file paths, simulation dimensions, and limits for various parameters. It centralizes configuration values, ensuring consistency and ease of maintenance across the codebase.

#### droneDataStructs
The droneDataStructs.h file defines key data structures of  force, pos, and velocity, used to represent the drone's physical properties. These structures facilitate the organization and manipulation of the drone's state within the simulation.

#### drone_parameters.json
The drone_parameters.json file provides configuration settings for the drone simulation, including parameters for the drone's physical properties and input controls. It allows for easy adjustment and tuning of simulation behavior through a structured JSON format.