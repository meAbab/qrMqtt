CC=g++
#ARCH=-arch armv6l

INCLUDE_FLAGS=-I/usr/local/include
CFLAGS=-Wall -ggdb -I./lib -I./lib/cpp
LDFLAGS=-L./lib ./lib/cpp/libmosquittopp.so.1 ./lib/libmosquitto.so.1 -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lzbar

.PHONY: all clean

all : qrMqtt

qrMqtt : main.o qrmosq.o qrcam.o
	${CC} $^ -o $@ ${LDFLAGS}

main.o : main.cpp
	${CC} -c $^ -o $@ ${CFLAGS}

qrmosq.o : qrmosq.cpp
	${CC} -c $^ -o $@ ${CFLAGS}

qrcam.o : qrcam.cpp
	${CC} ${ARCH} -c $^ -o $@ ${CFLAGS}
	
clean : 
	-rm -f *.o qrMqtt

