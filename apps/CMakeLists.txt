set(EXECUTABLE "Run")
add_executable(${EXECUTABLE} Run.cpp)
target_link_libraries(${EXECUTABLE} ${LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin
)

add_subdirectory(Task_1)
add_subdirectory(Task_2)
add_subdirectory(Task_3)
add_subdirectory(Task_4)
add_subdirectory(Task_5)
add_subdirectory(Task_6)
# add_subdirectory(Task_7)
add_subdirectory(Task_8)
add_subdirectory(Task_9)