project(ol_gl LANGUAGES C CXX)

set(OL_GL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(OL_GL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ol_camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ol_shader.cpp
)

set(OL_GLAD_ROOT "" CACHE PATH "Root directory containing glad/include and glad/src or glad libraries")
set(OL_GLM_ROOT "" CACHE PATH "Root directory containing the glm headers")

find_package(OpenGL REQUIRED)

find_package(glad CONFIG QUIET)
if(TARGET glad::glad)
    set(OL_GLAD_TARGET glad::glad)
elseif(TARGET glad)
    set(OL_GLAD_TARGET glad)
else()
    find_path(OL_GLAD_INCLUDE_DIR
        NAMES glad/glad.h
        HINTS ${OL_GLAD_ROOT} $ENV{GLAD_ROOT}
        PATH_SUFFIXES include
    )
    find_library(OL_GLAD_LIBRARY
        NAMES glad
        HINTS ${OL_GLAD_ROOT} $ENV{GLAD_ROOT}
        PATH_SUFFIXES lib lib64 lib-mingw-w64
    )
    find_file(OL_GLAD_SOURCE
        NAMES glad.c
        HINTS ${OL_GLAD_ROOT} $ENV{GLAD_ROOT}
        PATH_SUFFIXES src
    )

    if(OL_GLAD_INCLUDE_DIR AND OL_GLAD_LIBRARY)
        add_library(ol_glad_dependency UNKNOWN IMPORTED)
        set_target_properties(ol_glad_dependency PROPERTIES
            IMPORTED_LOCATION "${OL_GLAD_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OL_GLAD_INCLUDE_DIR}"
        )
    elseif(OL_GLAD_INCLUDE_DIR AND OL_GLAD_SOURCE)
        add_library(ol_glad_dependency STATIC "${OL_GLAD_SOURCE}")
        set_target_properties(ol_glad_dependency PROPERTIES POSITION_INDEPENDENT_CODE ON)
        target_include_directories(ol_glad_dependency PUBLIC "${OL_GLAD_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR
            "ol_gl requires GLAD. Set OL_GLAD_ROOT (or GLAD_ROOT) to a directory containing include/glad/glad.h and either src/glad.c or a glad library.")
    endif()
    set(OL_GLAD_TARGET ol_glad_dependency)
endif()

find_package(glm CONFIG QUIET)
if(TARGET glm::glm)
    set(OL_GLM_TARGET glm::glm)
else()
    find_path(OL_GLM_INCLUDE_DIR
        NAMES glm/glm.hpp
        HINTS ${OL_GLM_ROOT} $ENV{GLM_ROOT}
        PATH_SUFFIXES include
    )
    if(NOT OL_GLM_INCLUDE_DIR)
        message(FATAL_ERROR "ol_gl requires GLM. Set OL_GLM_ROOT (or GLM_ROOT) to the directory containing glm/glm.hpp.")
    endif()
    add_library(ol_glm_dependency INTERFACE)
    target_include_directories(ol_glm_dependency INTERFACE "${OL_GLM_INCLUDE_DIR}")
    set(OL_GLM_TARGET ol_glm_dependency)
endif()

set(OL_GL_LIBRARY_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}_${ARCHITECTURE}")

function(ol_configure_gl_target target_name)
    target_include_directories(${target_name} PUBLIC $<BUILD_INTERFACE:${OL_GL_INCLUDE_DIR}>)
    target_link_libraries(${target_name} PUBLIC ${OL_GLAD_TARGET} ${OL_GLM_TARGET} OpenGL::GL)
    set_target_properties(${target_name} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endfunction()

if(OL_BUILD_STATIC_LIBS)
    set(OL_GL_STATIC_DIR "${OL_GL_LIBRARY_BASE_DIR}_static")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_GL_STATIC_DIR "${OL_GL_STATIC_DIR}-${CMAKE_BUILD_TYPE}")
    endif()
    add_library(ol_gl_static STATIC ${OL_GL_SOURCES})
    set_target_properties(ol_gl_static PROPERTIES
        OUTPUT_NAME "ol_gl"
        ARCHIVE_OUTPUT_DIRECTORY "${OL_GL_STATIC_DIR}"
    )
    ol_configure_gl_target(ol_gl_static)
endif()

if(OL_BUILD_SHARED_LIBS)
    set(OL_GL_SHARED_DIR "${OL_GL_LIBRARY_BASE_DIR}_shared")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_GL_SHARED_DIR "${OL_GL_SHARED_DIR}-${CMAKE_BUILD_TYPE}")
    endif()
    add_library(ol_gl_shared SHARED ${OL_GL_SOURCES})
    set_target_properties(ol_gl_shared PROPERTIES
        OUTPUT_NAME "ol_gl"
        LIBRARY_OUTPUT_DIRECTORY "${OL_GL_SHARED_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${OL_GL_SHARED_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${OL_GL_SHARED_DIR}"
    )
    ol_configure_gl_target(ol_gl_shared)
endif()

if(TARGET ol_gl_static)
    add_library(ol_gl ALIAS ol_gl_static)
elseif(TARGET ol_gl_shared)
    add_library(ol_gl ALIAS ol_gl_shared)
endif()

if(OL_GL_WITH_TESTS)
    add_subdirectory(test)
endif()
