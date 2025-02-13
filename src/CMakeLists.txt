project("ARP_assignments")


# Set C++ Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Searching for curses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    /usr/local/include/fastdds
    /usr/local/include/fastcdr
    ${CMAKE_SOURCE_DIR}/include
)

# Define source files
set(SOURCE_TARGET_FILES 
    target.cpp
    ${CMAKE_SOURCE_DIR}/include/Generated/TargetTypeObjectSupport.cxx
    ${CMAKE_SOURCE_DIR}/include/Generated/TargetPubSubTypes.cxx
)

set(SOURCE_OBSTACLE_FILES 
    obstacle.cpp
    ${CMAKE_SOURCE_DIR}/include/Generated/ObstacleTypeObjectSupport.cxx
    ${CMAKE_SOURCE_DIR}/include/Generated/ObstaclePubSubTypes.cxx
)

set(SOURCE_SERVER_FILES 
    server.cpp
    ${CMAKE_SOURCE_DIR}/include/Generated/ObstacleTypeObjectSupport.cxx
    ${CMAKE_SOURCE_DIR}/include/Generated/ObstaclePubSubTypes.cxx
    ${CMAKE_SOURCE_DIR}/include/Generated/TargetTypeObjectSupport.cxx
    ${CMAKE_SOURCE_DIR}/include/Generated/TargetPubSubTypes.cxx
)

# Adding executables with relative files
add_executable(master master.c)
add_executable(server ${SOURCE_SERVER_FILES})
add_executable(drone drone.c)
add_executable(map map.c)
add_executable(watchdog watchdog.c)
add_executable(input input.c)
add_executable(target ${SOURCE_TARGET_FILES})
add_executable(obstacle ${SOURCE_OBSTACLE_FILES})

# Adding the required libraries for the executables
target_link_libraries(master wrappers constants)
target_link_libraries(server wrappers_cpp constants utility_cpp stdc++ fastcdr fastdds)
target_link_libraries(drone wrappers constants utility m)
target_link_libraries(map wrappers constants m utility ${CURSES_LIBRARIES})
target_link_libraries(watchdog wrappers constants utility)
target_link_libraries(input wrappers constants dronedatastructs utility m ${CURSES_LIBRARIES})
target_link_libraries(target wrappers_cpp constants utility_cpp stdc++ fastcdr fastdds)
target_link_libraries(obstacle wrappers_cpp constants utility_cpp stdc++ fastcdr fastdds)