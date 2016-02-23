# Bare bones description file (Makefile) for OpenVMS

PACKAGE = expat
VERSION = 1.95.8
EXPAT_MAJOR_VERSION=1
EXPAT_MINOR_VERSION=95
EXPAT_EDIT=8

O = .obj
OLB = .olb
 
LIBRARY = expat$(OLB)
LIBDIR = [.lib]
SOURCES = $(LIBDIR)xmlparse.c $(LIBDIR)xmltok.c $(LIBDIR)xmlrole.c
OBJECTS = xmlparse$(O) xmltok$(O) xmlrole$(O)
 
TEMPLATES = xmltok_impl.c xmltok_ns.c
APIHEADER = $(LIBDIR)expat.h
HEADERS = $(LIBDIR)ascii.h $(LIBDIR)iasciitab.h $(LIBDIR)utf8tab.h $(LIBDIR)xmltok.h \
	$(LIBDIR)asciitab.h $(LIBDIR)latin1tab.h \
	$(LIBDIR)nametab.h $(LIBDIR)xmldef.h $(LIBDIR)xmlrole.h $(LIBDIR)xmltok_impl.h
 
CONFIG_HEADER = expat_config.h
INCLUDES = /INCLUDE=([],[.lib])
DEFS = /DEFINE=(PACKAGE="""$(PACKAGE)""",VERSION="""$(PACKAGE)_$(VERSION)""",HAVE_EXPAT_CONFIG_H)
LIBREVISION = 0
LIBCURRENT  = 1
LIBAGE      = 0
# 
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
# 
# DISTFILES = $(DIST_COMMON) $(SOURCES) $(TEMPLATES) $(APIHEADER) $(HEADERS) 
# 
# TAR = gtar
# GZIP_ENV = --best
# 
.FIRST :
       IF F$SEARCH("$(LIBRARY)") .EQS. "" THEN $(LIBR) /CREATE /OBJECT $(LIBRARY)

all : $(LIBRARY)
        @ write sys$output "All made."
 
.SUFFIXES : 
.SUFFIXES : $(OLB) $(O) .C .H  
 
.c$(O) :
       $(COMPILE) $(MMS$SOURCE)
 
$(O)$(OLB) :
        @ IF F$SEARCH("$(MMS$TARGET)") .EQS. "" -
                THEN LIBRARY/CREATE/LOG $(MMS$TARGET)
        @ LIBRARY /REPLACE /LOG $(MMS$TARGET) $(MMS$SOURCE)
 
clean :
       DELETE $(LIBRARY);*,*$(O);*
 
$(LIBRARY) : $(LIBRARY)( $(OBJECTS) ) 
       $(LIBR) /COMPRESS $(MMS$TARGET)

$(CONFIG_HEADER) : [.vms]expat_config.h
        COPY/LOG $(MMS$SOURCE) $(MMS$TARGET)

xmlparse$(O) : $(LIBDIR)xmlparse.c $(LIBDIR)expat.h $(LIBDIR)xmlrole.h $(LIBDIR)xmltok.h $(CONFIG_HEADER)
 
xmlrole$(O) : $(LIBDIR)xmlrole.c $(LIBDIR)ascii.h $(LIBDIR)xmlrole.h $(CONFIG_HEADER)
 
xmltok$(O) : $(LIBDIR)xmltok.c $(LIBDIR)xmltok_impl.c $(LIBDIR)xmltok_ns.c \
        $(LIBDIR)ascii.h $(LIBDIR)asciitab.h $(LIBDIR)iasciitab.h $(LIBDIR)latin1tab.h \
	$(LIBDIR)nametab.h $(LIBDIR)utf8tab.h $(LIBDIR)xmltok.h $(LIBDIR)xmltok_impl.h $(CONFIG_HEADER)

