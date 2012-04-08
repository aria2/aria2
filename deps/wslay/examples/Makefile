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
CC = gcc
CXX = g++
CFLAGS = -g -O2 -I../lib/includes
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L../lib/.libs
LIBS = -lwslay -lnettle

PROGRAMS = fork-echoserv echoserv testclient

.PHONY: all
all: $(PROGRAMS)

fork-echoserv: fork-echoserv.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ fork-echoserv.c $(LIBS)

echoserv: echoserv.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ echoserv.cc $(LIBS)

testclient: testclient.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ testclient.cc $(LIBS)

.PHONY: clean
clean:
	rm -f *.o $(PROGRAMS)
