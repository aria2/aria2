
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
	set(numToSubstract 15)  # this value here, has been set based on the biggest string that currently exists from the ones that needs to be printed
	string(LENGTH ${value} sizeOfVar)
	math(EXPR numSpaces "${numToSubstract} - ${sizeOfVar}")

	foreach(space RANGE 0 ${numSpaces})
		string(APPEND spaces " ")
	endforeach()

	return(PROPAGATE spaces)
endfunction()
