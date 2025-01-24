project("ARP_assignments")

# Searching for curses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Adding executables with realative files
add_executable(master master.c)
add_executable(server server.c)
add_executable(drone drone.c)
add_executable(map map.c)
add_executable(WD WD.c)
add_executable(input input.c)
add_executable(target target.c)
add_executable(obstacles obstacles.c)

# Adding the required libraries for the executables
target_link_libraries(master wrapFunc constants)
target_link_libraries(server wrapFunc constants utils)
target_link_libraries(drone wrapFunc constants utils m)
target_link_libraries(map wrapFunc constants m utils ${CURSES_LIBRARIES})
target_link_libraries(WD wrapFunc constants utils)
target_link_libraries(input wrapFunc constants datastructs utils m ${CURSES_LIBRARIES})
target_link_libraries(target wrapFunc constants utils)
target_link_libraries(obstacles wrapFunc constants utils)
