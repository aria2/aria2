@REM make_example.bat
@REM
@REM Create Makefile from project file and then make QtTestRunner example.

qmake qt_example.pro
nmake distclean
nmake
