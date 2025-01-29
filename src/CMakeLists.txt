project("ARP_assignments")

# Searching for curses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Adding executables with realative files
add_executable(master master.c)
add_executable(server server.c)
add_executable(drone drone.c)
add_executable(map map.c)
add_executable(watchdog watchdog.c)
add_executable(input input.c)
add_executable(target target.c)
add_executable(obstacle obstacle.c)

# Adding the required libraries for the executables
target_link_libraries(master wrappers constants)
target_link_libraries(server wrappers constants utility)
target_link_libraries(drone wrappers constants utility m)
target_link_libraries(map wrappers constants m utility ${CURSES_LIBRARIES})
target_link_libraries(watchdog wrappers constants utility)
target_link_libraries(input wrappers constants dronedatastructs utility m ${CURSES_LIBRARIES})
target_link_libraries(target wrappers constants utility)
target_link_libraries(obstacle wrappers constants utility)
