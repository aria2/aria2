#!/bin/sh
# Look for program[s] somewhere in $PATH.
#
# Options:
#  -s
#    Do not print out full pathname. (silent)
#  -pPATHNAME
#    Look in PATHNAME instead of $PATH
#
# Usage:
#  PrintPath [-s] [-pPATHNAME] program [program ...]
#
# Initially written by Jim Jagielski for the Apache configuration mechanism
#  (with kudos to Kernighan/Pike)
#
# This script falls under the Apache License.
# See http://www.apache.org/licenses/LICENSE

##
# Some "constants"
##
pathname=$PATH
echo="yes"

##
# Find out what OS we are running for later on
##
os=`(uname) 2>/dev/null`

##
# Parse command line
##
for args in $*
do
    case $args in
	-s  ) echo="no" ;;
	-p* ) pathname="`echo $args | sed 's/^..//'`" ;;
	*   ) programs="$programs $args" ;;
    esac
done

##
# Now we make the adjustments required for OS/2 and everyone
# else :)
#
# First of all, all OS/2 programs have the '.exe' extension.
# Next, we adjust PATH (or what was given to us as PATH) to
# be whitespace separated directories.
# Finally, we try to determine the best flag to use for
# test/[] to look for an executable file. OS/2 just has '-r'
# but with other OSs, we do some funny stuff to check to see
# if test/[] knows about -x, which is the preferred flag.
##

if [ "x$os" = "xOS/2" ]
then
    ext=".exe"
    pathname=`echo -E $pathname |
     sed 's/^;/.;/
	  s/;;/;.;/g
	  s/;$/;./
	  s/;/ /g
	  s/\\\\/\\//g' `
    test_exec_flag="-r"
else
    ext=""	# No default extensions
    pathname=`echo $pathname |
     sed 's/^:/.:/
	  s/::/:.:/g
	  s/:$/:./
	  s/:/ /g' `
    # Here is how we test to see if test/[] can handle -x
    testfile="pp.t.$$"

    cat > $testfile <<ENDTEST
#!/bin/sh
if [ -x / ] || [ -x /bin ] || [ -x /bin/ls ]; then
    exit 0
fi
exit 1
ENDTEST

    if `/bin/sh $testfile 2>/dev/null`; then
	test_exec_flag="-x"
    else
	test_exec_flag="-r"
    fi
    rm -f $testfile
fi

for program in $programs
do
    for path in $pathname
    do
	if [ $test_exec_flag $path/${program}${ext} ] && \
	   [ ! -d $path/${program}${ext} ]; then
	    if [ "x$echo" = "xyes" ]; then
		echo $path/${program}${ext}
	    fi
	    exit 0
	fi

# Next try without extension (if one was used above)
	if [ "x$ext" != "x" ]; then
            if [ $test_exec_flag $path/${program} ] && \
               [ ! -d $path/${program} ]; then
                if [ "x$echo" = "xyes" ]; then
                    echo $path/${program}
                fi
                exit 0
            fi
        fi
    done
done
exit 1

