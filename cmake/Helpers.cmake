
# print in a nice way the values of the build that the user chose
function(printBuildOptions keys values)
	foreach(elemKey elemValue IN ZIP_LISTS "${keys}" "${values}")
		set(spaces)
		spaceIdentation(${elemKey})
		message("${elemKey}:${spaces}${elemValue}")
		unset(spaces)
	endforeach()
endfunction()


function(spaceIdentation value)
	set(numToSubstract 22)  # this value here, has been set based on the biggest string that currently exists from the ones that needs to be printed
	string(LENGTH ${value} sizeOfVar)
	math(EXPR numSpaces "${numToSubstract} - ${sizeOfVar}")

	foreach(space RANGE 0 ${numSpaces})
		string(APPEND spaces " ")
	endforeach()

	return(PROPAGATE spaces)
endfunction()


# custom function, that creates config.h file at CMAKE_BUILD_BINARY_DIR that contains the definitions
# from CMakeLists.txt files
# try to imitate in a nice way the AC_CONFIG_HEADERS() from autotools build tool
function(writeToConfigFile definition value message)
    set(content [==[
/* @message@ */
]==])

    if(value)
        set(content_p [==[
#define @definition@ @value@

]==])
        string(APPEND content "${content_p}")
        string(CONFIGURE "${content}" content @ONLY)
    else()
        set(content_p [==[
/* #undef @definition@ */

]==])
        string(APPEND content "${content_p}")
        string(CONFIGURE "${content}" content @ONLY)
    endif()

        file(APPEND "${PROJECT_SOURCE_DIR}/src/config.h" ${content})

endfunction()
