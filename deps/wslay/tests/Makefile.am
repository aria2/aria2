# Wslay - The WebSocket Library

# Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa

# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:

# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

if HAVE_CUNIT

check_PROGRAMS = main

OBJECTS = main.c wslay_frame_test.c\
	wslay_event_test.c\
	wslay_queue_test.c

HFILES = wslay_session_test.h wslay_frame_test.h\
	wslay_event_test.h\
	wslay_queue_test.h

main_SOURCES = $(HFILES) $(OBJECTS)

main_LDADD = ${top_builddir}/lib/libwslay.la -lcunit
main_LDFLAGS = -static
AM_CFLAGS = -Wall -g -O2 -I${top_srcdir}/lib -I${top_srcdir}/lib/includes\
	@DEFS@
# DEFS += -D_ISOC99_SOURCE -D_GNU_SOURCE
TESTS = main

endif # HAVE_CUNIT
