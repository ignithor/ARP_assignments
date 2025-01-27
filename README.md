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

### Rules of the game

TODO @Yui

The score increment for the current game is calculated with the following formula:

```math
score\_increment =
\begin{cases}
    15 - ⌊t⌋, & \text{if } t < 15 \\
    4, & \text{if } t \geq 20 for target 1
\end{cases}
```

where ⌊t⌋ is the time taken by the drone to reach the target starting from the instant when the targets are spawned in the map.

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

