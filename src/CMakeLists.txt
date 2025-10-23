add_library(practice)

add_subdirectory(blocking)
add_subdirectory(demo)

target_include_directories(practice PUBLIC ${MAIN_INCLUDE_DIR})