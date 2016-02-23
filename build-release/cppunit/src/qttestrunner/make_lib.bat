@REM make_lib.bat
@REM
@REM Create Makefile from project file and then make QtTestRunner library.

qmake qttestrunnerlib.pro
nmake distclean
nmake
