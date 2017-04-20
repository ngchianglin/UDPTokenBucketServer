#
# MIT License
#
# Copyright (c) 2017 Ng Chiang Lin
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# 
# Makefile to build the 
# upd token bucket server
# Ng Chiang Lin
# April 2017
#

CC=cc
CFLAGS= -Wall -Wextra -Wformat=2 -fPIE -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-strong 
OFLAGS= -c 
LFLAGS= -lpthread -Wl,-z,relro -Wl,-z,now -Wl,--strip-all
OBJS=bucketserver.o hashtable.o queue.o ip4bucket.o 

all: tbserver

tbserver: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o tbserver $(LFLAGS)

%.o : %.c ratelimit.h
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<

testclient: testclient.c
	$(CC) $(CFLAGS) $< -o testclient $(LFLAGS)

clean:
	rm -f tbserver 
	rm -f testclient
	rm -f *.o
	rm -f *.gch

.PHONY:all clean

