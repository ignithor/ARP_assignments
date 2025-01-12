project("ARP_assignment_1")

# Searching for curses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})


include_directories("/usr/include")

find_library(CJSON_LIB cjson REQUIRED)
message(STATUS "cJSON library found at: ${CJSON_LIB}")


# Setting macros for files
set(WRAP_FUNC_FILES
    ../include/wrapFunc.h
    wrapFunc.c)

set(UTILS_FILES
    ../include/utils.h
    utils.c)

# Setting libraries names for those files
add_library(wrapFunc ${WRAP_FUNC_FILES})
add_library(utils ${UTILS_FILES})

# setting the building interface in order to have a correct include interface
target_include_directories(
    wrapFunc
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )

target_include_directories(
    utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE /usr/include
    )

target_link_libraries(utils PRIVATE ${CJSON_LIB})
target_link_libraries(wrapFunc utils)



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
target_link_libraries(master wrapFunc constants)
target_link_libraries(server wrapFunc constants utils)
target_link_libraries(drone wrapFunc constants utils m)
target_link_libraries(map wrapFunc constants m utils ${CURSES_LIBRARIES})
target_link_libraries(watchdog wrapFunc constants utils)
target_link_libraries(input wrapFunc constants datastructs utils m ${CURSES_LIBRARIES})
target_link_libraries(target wrapFunc constants utils)
target_link_libraries(obstacle wrapFunc constants utils)
