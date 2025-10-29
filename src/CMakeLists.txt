add_library(practice)

add_subdirectory(blocking)
add_subdirectory(demo)
add_subdirectory(utils)

target_include_directories(practice PUBLIC ${MAIN_INCLUDE_DIR})
