project("ARP_Assignment_1")

# Adding header only libraries
add_library(constants INTERFACE)
add_library(datastructs INTERFACE)

# Adding files for those libraries
target_include_directories(
    constants
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )

target_include_directories(
    datastructs
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    )
