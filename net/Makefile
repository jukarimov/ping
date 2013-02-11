all: ping pong server

LIBS = -lcurses 
CFLAGS = -Wall -g -O2

SRCS1 = ping.c
SRCS2 = pong.c

ping:
	cc $(CFLAGS) $(SRCS1) -o $@ $(LIBS)

pong:
	cc $(CFLAGS) $(SRCS2) -o $@ $(LIBS)

server:
	cc $(CFLAGS) server.c -o $@

clean:
	rm -rf ping pong server
