CC=g++

all: isamon

isamon: isamon.cpp
	$(CC) isamon.cpp -o isamon

clean:
	rm -fr isamon