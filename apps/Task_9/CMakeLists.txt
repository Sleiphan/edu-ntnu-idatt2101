set(EXECUTABLE "Task_9")
add_executable(${EXECUTABLE} Oving_9.cpp)
target_link_libraries(${EXECUTABLE} ${LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin
)