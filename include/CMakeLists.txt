project("ARP_assignments")

include_directories("/usr/include")

find_library(CJSON_LIB cjson REQUIRED)
message(STATUS "cJSON library found at: ${CJSON_LIB}")

# Setting macros for files
set(WRAP_FUNC_FILES
    wrappers/wrappers.h
    wrappers/wrappers.c)

set(WRAP_CPP_FUNC_FILES
    wrappers/wrappers.hpp
    wrappers/wrappers.cpp)

set(UTILS_FILES
    utility/utility.h
    utility/utility.c)

set(UTILS_CPP_FILES
    utility/utility.hpp
    utility/utility.cpp)

# Setting libraries names for those files
add_library(wrappers ${WRAP_FUNC_FILES})
add_library(wrappers_cpp ${WRAP_CPP_FUNC_FILES})
add_library(utility ${UTILS_FILES})
add_library(utility_cpp ${UTILS_CPP_FILES})



# setting the building interface in order to have a correct include interface
target_include_directories(
    wrappers
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )

target_include_directories(
    wrappers_cpp
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )

target_include_directories(
    utility
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE /usr/include
    )

target_include_directories(
    utility_cpp
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE /usr/include
    )

target_link_libraries(utility PRIVATE ${CJSON_LIB})
target_link_libraries(utility_cpp PRIVATE ${CJSON_LIB} wrappers_cpp)

target_link_libraries(wrappers utility)
target_link_libraries(wrappers_cpp utility_cpp)


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
