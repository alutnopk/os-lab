CC = g++
CFLAGS = -Wall
RM = rm -f

guest: guest.cpp headers.h
	$(CC) -I. guest.cpp -o guest
cleaner: cleaner.cpp headers.h
	$(CC) -I. cleaner.cpp -o cleaner
main: main.cpp guest cleaner headers.h
	$(CC) -I. main.cpp -o main
run: main
	./main
clean:
	$(RM) main guest cleaner