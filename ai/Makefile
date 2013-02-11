
OBJS = ping.o

LIBS = -lncurses 
CFLAGS = -Wall -g -O2

ping: $(OBJS)
	cc $< $(LIBS) -o $@

clean:
	rm -rf *.o ping
