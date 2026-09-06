project(ol_qt_tests LANGUAGES CXX)

add_executable(test_ol_media_playlist test_ol_media_playlist.cpp)
target_link_libraries(test_ol_media_playlist PRIVATE ol_qt)

if(CMAKE_CONFIGURATION_TYPES)
    set(OL_QT_TEST_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}")
else()
    set(OL_QT_TEST_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}-${CMAKE_BUILD_TYPE}")
endif()
set_target_properties(test_ol_media_playlist PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OL_QT_TEST_BIN_DIR}")

add_test(NAME test_ol_media_playlist COMMAND test_ol_media_playlist)
set_tests_properties(test_ol_media_playlist PROPERTIES LABELS "qt;automatic" TIMEOUT 10)
