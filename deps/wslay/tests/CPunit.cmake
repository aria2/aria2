if(HAVE_CUNIT)
    add_executable(main)

    target_sources(main
        PRIVATE
            tests/main.c
            tests/wslay_frame_test.c
	    tests/wslay_event_test.c
            tests/wslay_queue_test.c

        PUBLIC
            FILE_SET HEADERS
#            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/tests
            FILES
                tests/wslay_session_test.h
                tests/wslay_frame_test.h
                tests/wslay_event_test.h
                tests/wslay_queue_test.h
    )


    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O2")


    message("MARINOS ${CMAKE_CURRENT_BINARY_DIR}")
 
   target_link_libraries(main PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/lib/libwslay.so
	${CUNIT_LIBRARIES}
    )

	message("The test source dir is ${CMAKE_CURRENT_SOURCE_DIR}")

    target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/includes
    )

endif()
