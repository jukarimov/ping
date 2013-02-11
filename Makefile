
all: ping-ai ping-mul ping-net

ping-ai:
	cd ai && make && cp ping ../ping-ai

ping-mul:
	cd mul && make && cp ping ../ping-mul

ping-net:
	cd net && make && \
	cp ping ../ping-net && \
	cp pong ../pong-net && \
	cp server ../ping-net-server

clean:
	rm -f ping-* pong-*
	cd ai && make clean
	cd mul && make clean
	cd net && make clean
