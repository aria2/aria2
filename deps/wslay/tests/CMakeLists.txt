if(HAVE_CUNIT)
    add_executable(main)
    
    target_sources(main 
        PRIVATE
            main.c
            wslay_frame_test.c
	        wslay_event_test.c
	        wslay_queue_test.c

        PUBLIC
            FILE_SET HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
            FILES
                wslay_session_test.h
                wslay_frame_test.h
	            wslay_event_test.h
	            wslay_queue_test.h
    )

    string(APPEND CMAKE_C_FLAGS " -Wall -g -O2")

    target_link_libraries(main PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/../lib/libwslay.so
        ${CUNIT_LIBRARIES}
    )

    target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/includes
    )

endif()