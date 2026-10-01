<<<<<<< HEAD
add_library(wslay STATIC)

# add_subdirectory(includes)
=======
add_library(wslay SHARED)
>>>>>>> 794a3fc9 (migrate websocket library to CMake)

target_compile_options(wslay PRIVATE
    -Wall
)

target_include_directories(wslay PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(wslay PROPERTIES
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_sources(wslay
    PRIVATE
        wslay_frame.c
        wslay_event.c
        wslay_queue.c
        wslay_net.c

        ${CMAKE_CURRENT_SOURCE_DIR}/includes/wslay/wslay.h
        ${CMAKE_CURRENT_SOURCE_DIR}/includes/wslay/wslayver.h

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            wslay_frame.h
            wslay_event.h
            wslay_queue.h
            wslay_net.h
            wslay_macro.h
)
