
This package contains:

 * the SQLite library amalgamation (single file) source code distribution,
 * the shell.c file used to build the sqlite3 shell too, and
 * the sqlite3.h and sqlite3ext.h header files required to link programs
   and sqlite extensions against the installed libary.
 * autoconf/automake installation infrastucture.

The generic installation instructions for autoconf/automake are found
in the INSTALL file.

The following SQLite specific boolean options are supported:

  --enable-readline           use readline in shell tool   [default=yes]
  --enable-threadsafe         build a thread-safe library  [default=yes]
  --enable-dynamic-extensions support loadable extensions  [default=yes]

The default value for the CFLAGS variable (options passed to the C 
compiler) includes debugging symbols in the build, resulting in larger
binaries than are necessary. Override it on the configure command
line like this:

  $ CFLAGS="-Os" ./configure

to produce a smaller installation footprint.

Other SQLite compilation parameters can also be set using CFLAGS. For
example:

  $ CFLAGS="-Os -DSQLITE_OMIT_TRIGGERS" ./configure

