CC=g++

APPNAME= nocnn

all: $(APPNAME)

$(APPNAME): main.o cnn.o noc.o estimation.o
	$(CC) -lm cnn.o noc.o estimation.o main.o -o $(APPNAME)

main.o: main.cpp cnn.h noc.h estimation.h
	$(CC) -c main.cpp

cnn.o: cnn.cpp cnn.h 
	$(CC) -c cnn.cpp

noc.o: noc.cpp noc.h 
	$(CC) -c noc.cpp

estimation.o: estimation.cpp estimation.h cnn.h noc.h
	$(CC) -c estimation.cpp

clean:
	rm -f *.o $(APPNAME)
