project(ol_database_tests LANGUAGES CXX)

find_package(Threads REQUIRED)

add_executable(test_ol_database_pool test_ol_database_pool.cpp)
target_include_directories(test_ol_database_pool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(test_ol_database_pool PRIVATE ol_core Threads::Threads)

if(CMAKE_CONFIGURATION_TYPES)
    set(OL_DATABASE_TEST_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}")
else()
    set(OL_DATABASE_TEST_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}-${CMAKE_BUILD_TYPE}")
endif()
set_target_properties(test_ol_database_pool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OL_DATABASE_TEST_BIN_DIR})

add_test(NAME test_ol_database_pool COMMAND test_ol_database_pool)
set_tests_properties(test_ol_database_pool PROPERTIES
    LABELS "database;automatic"
    TIMEOUT 10
)
