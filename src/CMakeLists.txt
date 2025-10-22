add_library(practice
  "hello_world.cpp"
)

target_include_directories(practice PUBLIC ${MAIN_INCLUDE_DIR})