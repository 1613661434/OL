project(ol_gl_tests LANGUAGES CXX)

add_executable(test_ol_shader_move test_ol_shader_move.cpp)
target_link_libraries(test_ol_shader_move PRIVATE ol_gl)

if(CMAKE_CONFIGURATION_TYPES)
    set(OL_GL_TEST_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}")
else()
    set(OL_GL_TEST_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}-${CMAKE_BUILD_TYPE}")
endif()
set_target_properties(test_ol_shader_move PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OL_GL_TEST_BIN_DIR}")

add_test(NAME test_ol_shader_move COMMAND test_ol_shader_move)
set_tests_properties(test_ol_shader_move PROPERTIES LABELS "gl;automatic" TIMEOUT 10)
