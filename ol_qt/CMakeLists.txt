project(ol_qt LANGUAGES CXX)

find_package(Qt6 6.0 REQUIRED COMPONENTS Core Multimedia)

set(OL_QT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(OL_QT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ol_MediaPlaylist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ol_MediaPlaylist.cpp
)
set(OL_QT_LIBRARY_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}_${ARCHITECTURE}")

function(ol_configure_qt_target target_name)
    target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${OL_QT_INCLUDE_DIR}>)
    target_link_libraries(${target_name} PUBLIC Qt6::Core Qt6::Multimedia)
    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endfunction()

if(OL_BUILD_STATIC_LIBS)
    set(OL_QT_STATIC_DIR "${OL_QT_LIBRARY_BASE_DIR}_static")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_QT_STATIC_DIR "${OL_QT_STATIC_DIR}-${CMAKE_BUILD_TYPE}")
    endif()
    add_library(ol_qt_static STATIC ${OL_QT_SOURCES})
    set_target_properties(ol_qt_static PROPERTIES
        OUTPUT_NAME "ol_qt"
        ARCHIVE_OUTPUT_DIRECTORY "${OL_QT_STATIC_DIR}"
    )
    ol_configure_qt_target(ol_qt_static)
endif()

if(OL_BUILD_SHARED_LIBS)
    set(OL_QT_SHARED_DIR "${OL_QT_LIBRARY_BASE_DIR}_shared")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_QT_SHARED_DIR "${OL_QT_SHARED_DIR}-${CMAKE_BUILD_TYPE}")
    endif()
    add_library(ol_qt_shared SHARED ${OL_QT_SOURCES})
    set_target_properties(ol_qt_shared PROPERTIES
        OUTPUT_NAME "ol_qt"
        LIBRARY_OUTPUT_DIRECTORY "${OL_QT_SHARED_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${OL_QT_SHARED_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${OL_QT_SHARED_DIR}"
    )
    ol_configure_qt_target(ol_qt_shared)
endif()

if(TARGET ol_qt_static)
    add_library(ol_qt ALIAS ol_qt_static)
elseif(TARGET ol_qt_shared)
    add_library(ol_qt ALIAS ol_qt_shared)
endif()

if(OL_QT_WITH_TESTS)
    add_subdirectory(test)
endif()
