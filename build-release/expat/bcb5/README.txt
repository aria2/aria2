
                   Using a Borland compiler product

The files in this directory support using both the free Borland command-line
compiler tools and the Borland C++ Builder IDE.  The project files have been
tested with both versions 5 and 6 of the C++ Builder product.

             Using the free BCC32 command line compiler

After downloading and installing the free C++ Builder commandline version,
perform the following steps (assuming it was installed under C:\Borland\BCC55):

1) Add "C:\Borland\BCC55\BIN" to your path
2) Set the environment variable BCB to "C:\Borland\BCC55".
3) edit makefile.mak: enable or comment out the appropriate commands under
   clean & distclean, depending on whether your OS can use deltree /y or
   del /s/f/q.

After that, you should simply cd to the bcb5 directory in your Expat directory
tree (same structure as CVS) and run "make all" or just "make".

                               Naming

The libraries have the base name "libexpat" followed optionally by an "s"
(static) or a "w" (unicode version), then an underscore and optionally
"mt" (multi-threaded) and "d" (dynamic RTL).

To change the name of the library a project file produces, edit the project
option source (see step 1 under Unicode below) and change the name contained in
the PROJECT tag. In a make file, change the value assigned to the PROJECT
variable. Also, the LIBRARY entry in the .def file has to be changed to
correspond to the new executable name.


                       Unicode Considerations

There are no facilities in the BCB 5 GUI to create a unicode-enabled
application. Fortunately, it is not hard to do by hand.

1. The startup .obj system file must be changed to the unicode version.
   Go to Project|Edit Option Source, and scroll down to the ALLOBJ tag. Change
   c0x32.obj to c0x32w.obj. Editing this file can be quirky, but usually the
   following kludge will make the change stick. Close and save the file
   (CTRL-F4) then open the options dialog (CTRL-Shift-F11), then click OK on
   the dialog immediately without changing anything in it. If this doesn't work,
   you will have to close the project completely and edit the .bpr file by hand.

   If you are using a make file, just change the startup .obj file assigned
   to the ALLOBJ variable.

2. Add the macro define XML_UNICODE_WCHAR_T. In the GUI that goes in the options
   dialog, Directories/Conditionals tab, in the Conditional define box. In a
   make file, put it in the USERDEFINES variable.

3. Of course, your code has to be written for unicode. As a start, the "main"
   function is called "wmain". The tchar macros are an interesting way to
   write code that can easily switch between unicode and utf-8. If these macros
   are used, then simply adding the conditional define _UNICODE as well as
   XML_UNICODE_WCHAR_T will bring in the unicode versions of the tchar macros.
   Otherwise the utf-8 versions are used. xmlwf uses its own versions of the
   tchar macros which are switched on and off by the XML_UNICODE macro, which
   itself is set by the XML_UNICODE_WCHAR_T define.

                              Threading

The libexpat libraries are all built to link with the multi-threaded dynamic RTL's.
That means they require CC32xxMT.DLL present on the installation target.
To create single-threaded libs, do the following:

1. The compiler option for multi-threading must be turned off. Following the
   instructions above to edit the option source, remove the -tWM option from
   the CFLAG1 tag. In a make file, remove it from the CFLAG1 variable.

2. The single threaded RTL must be called. change the RTL in the ALLLIB tag or
   variable (GUI or makefile repectively) to the version without the "mt" in the
   name. For example, change cw32mti.lib to cw32i.lib.

                              Static RTL's

To build the libs with static RTL's do the following,

1. For the static expatlibs, in the Tlib tab on the options dialog, uncheck the
   "Use dynamic RTL" box. For the dynamic expatlibs, in the Linker tab on the
   options dialog, uncheck "Use dynamic RTL". If you are using a make file,
   remove the _RTLDLL assignment to the SYSDEFINES variable, and change the RTL
   to the version without an "i" in the ALLLIB variable. For example,
   cw32mti.lib would become cw32mt.lib.
