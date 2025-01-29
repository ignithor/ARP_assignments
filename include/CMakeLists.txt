project("ARP_assignments")

include_directories("/usr/include")

find_library(CJSON_LIB cjson REQUIRED)
message(STATUS "cJSON library found at: ${CJSON_LIB}")

# Setting macros for files
set(WRAP_FUNC_FILES
    wrappers/wrappers.h
    wrappers/wrappers.c)

set(UTILS_FILES
    utility/utility.h
    utility/utility.c)

# Setting libraries names for those files
add_library(wrappers ${WRAP_FUNC_FILES})
add_library(utility ${UTILS_FILES})

# setting the building interface in order to have a correct include interface
target_include_directories(
    wrappers
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )

target_include_directories(
    utility
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE /usr/include
    )

target_link_libraries(utility PRIVATE ${CJSON_LIB})
target_link_libraries(wrappers utility)

# Adding header only libraries
add_library(constants INTERFACE)
add_library(dronedatastructs INTERFACE)

# Adding files for those libraries
target_include_directories(
    constants
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )

target_include_directories(
    dronedatastructs
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )
