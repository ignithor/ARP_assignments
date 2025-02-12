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
- **If \( t <= 20 \):**
  - If the target number is 1:  
    `score_increment = 4 + (20 - t)`
  - If the target number is not 1:  
    `score_increment = 2`
- **If \( t > 20 \):**
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
|  t < 20      | 1             | 4 + (20 - t)   |
|  t < 20      | Not 1         | +2       |
|  t >= 20  | 1             | +4              |
|  t >= 20  | Not 1         | +2              |
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



#### Target

#### Obstacle
