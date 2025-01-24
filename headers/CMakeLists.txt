project("ARP_assignments")

include_directories("/usr/include")

find_library(CJSON_LIB cjson REQUIRED)
message(STATUS "cJSON library found at: ${CJSON_LIB}")

# Setting macros for files
set(WRAP_FUNC_FILES
    wrapFuncs/wrapFunc.h
    wrapFuncs/wrapFunc.c)

set(UTILS_FILES
    utils/utils.h
    utils/utils.c)

# Setting libraries names for those files
add_library(wrapFunc ${WRAP_FUNC_FILES})
add_library(utils ${UTILS_FILES})

# setting the building interface in order to have a correct include interface
target_include_directories(
    wrapFunc
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../headers>
    )

target_include_directories(
    utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../headers>
    PRIVATE /usr/include
    )

target_link_libraries(utils PRIVATE ${CJSON_LIB})
target_link_libraries(wrapFunc utils)

# Adding header only libraries
add_library(constants INTERFACE)
add_library(datastructs INTERFACE)

# Adding files for those libraries
target_include_directories(
    constants
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../headers>
    )

target_include_directories(
    datastructs
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../headers>
    )
