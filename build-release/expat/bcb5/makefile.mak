all: setup expat expatw expat_static expatw_static elements outline xmlwf

setup:
 setup

expat:
 make -l -fexpat.mak

expatw:
 make -l -fexpatw.mak

expat_static:
 make -l -fexpat_static.mak

expatw_static:
 make -l -fexpatw_static.mak

elements:
 make -l -felements.mak

outline:
 make -l -foutline.mak

xmlwf:
 make -l -fxmlwf.mak

clean:
# works on Win98/ME
# deltree /y release\obj
# works on WinNT/2000
 del /s/f/q release\obj

distclean:
# works on Win98/ME
# deltree /y release\*.*
# works on WinNT/2000
 del /s/f/q release\*
