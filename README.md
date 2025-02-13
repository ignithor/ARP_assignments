# ARP_assignments

# Authors

Yui MOMIYAMA

Paul PHAM DANG

Group 3

## How to run

### Dependencies

These dependencies are needed:

- make
- CMake version > 3.10
- libncurses

    sudo apt install libncurses5-dev

- lib cjson

    sudo apt install libcjson-dev

- FastDDS (3.1.0)

Download FastDDS Binaries

    curl -o fastdds.tgz '<https://www.eprosima.com/component/ars/item/eProsima_Fast-DDS-v3.1.0-Linux.tgz?format=tgz&category_id=7&release_id=169&Itemid=0'>

Unzip fastdds.tgz

    mkdir fastdds
    tar -xvzf ./fastdds.tgz -C ./fastdds
    cd fastdds
    sudo ./install.sh

The assignment has been tried using Ubuntu 22.04

### Run the assignments

Simply execute the scripts by executing:

For the assignment 1 :

    ./run_assignment1.sh

For assignment 2 :

    ./run_assignment2.sh

To change from assignment 1 and 2, you can do

    git checkout <assignment>

There are two branches :
main is assignment 1 and assignment2 is for assignment 2.
If you use the script it will automatically change to the right branch

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

### Software architecture of the second assignment

![plot](/docs/architecture2.jpg)

In the second assignment, the project is executed on two computers using the same LAN. Each computer runs a copy of the assignment 2. In the constants.h file, we defined a constant called PUBLISHERS_SLEEP_MODE, when this constant is set to true, the Target and Obstacle publishers are put on a sleep mode where they are publishing nothing. So one computer can run it setting this constant to true and the other to false.
Communication between them is handled via DDS.

As illustrated in the figure, on Computer 1, the operator controls the drone, while the processes Obstacle and Target on Computer 2 generate obstacles and targets.
However, these roles are interchangeable, allowing the operator on Computer 2 to control the drone while obstacles and targets are generated by processes Obstacle and Target on Computer 1.

In order to use fastDDS, we use C++ for some files of this assignment. So the files target, obstacle and server are now written in C++. Moreover, the libraries wrappers and utility had to be changed.

#### target

The target.cpp file defines a `TargetPublisher` class to manage the publishing of Target messages using the Fast DDS library. During the 5 first seconds the process publish the initial targets. Then, after OBSTACLES_SPAWN_PERIOD * 5 , it changes the targets' position and publish the new coordinates.

#### obstacle

The obstacle.cpp file defines a `ObstaclePublisher` class to manage the publishing of Object messages every OBSTACLES_SPAWN_PERIOD using the Fast DDS library.

#### server

In the server, we add the TargetSubscriber and ObstacleSubscriber classes. These classes subscribe to Fast DDS topics for target and obstacle data, respectively. They initialize participants, subscribers, topics, and data readers, and process incoming messages to extract coordinates. Processed data is forwarded via pipes to other components (e.g., drone and map). Nothing changed for the others pipes.

### Other related files

#### Target.idl

The Target.idl file defines a `Target` structure with two arrays, `target_x` and `target_y`, each containing 9 double values. This structure specifies the data format for target coordinates in a DDS system.

#### Obstacle.idl

The Obstacle.idl file defines a `Obstacle` structure with two arrays, `obstacle_x` and `obstacle_y`, each containing 9 double values. This structure specifies the data format for target coordinates in a DDS system.

#### constants.h

Here we defined three new constants compared to assignment 1 :

- TOPIC_NAME_TARGET
- TOPIC_NAME_OBSTACLE
- PUBLISHERS_SLEEP_MODE

And the constants N_OBSTACLES and N_TARGETS can't be changed easily in this assignment because their numbers are hard coded in the idl files. So if we want to change the number of obstacle for example, you have to change this constant and also change it in the Obstacle.idl file. Then, you have to generate again the message type using :

    fastddsgen ./Obstacle.idl -d ./Generated

in the include folder.

### Remark

As we deleted the pipe from Server to Target and Server to Obstacle, when we press `P` the server doesn't send the message to this two processes. The process are killed by the Watchdog which is not very elegant. But as we should work with two different computers we can't use a pipe from one computer to another. But we could add an another topic to do that but it wasn't in the assignment guidelines.
