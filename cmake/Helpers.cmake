
# print in a nice way the values of the build that the user chose
function(printBuildOptions keys values)
	foreach(elemKey elemValue IN ZIP_LISTS "${keys}" "${values}")
		message("${elemKey}:      ${elemValue}")
	endforeach()
endfunction()
