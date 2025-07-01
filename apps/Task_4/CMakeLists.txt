set(EXECUTABLE "Task_4")
add_executable(${EXECUTABLE} Oving_4.cpp)
target_link_libraries(${EXECUTABLE} ${LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin
)
