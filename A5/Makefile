CC = g++
CFLAGS = -Wall -Wextra
RM = rm -f
HEADERS = headers.h
MAIN = main.cpp
GUEST = guest.cpp
CLEANER = cleaner.cpp

guest: $(GUEST) $(HEADERS)
	$(CC) $(CFLAGS) -I. $(GUEST) -o guest
cleaner: $(CLEANER) $(HEADERS)
	$(CC) $(CFLAGS) -I. $(CLEANER) -o cleaner
main: $(MAIN) guest cleaner $(HEADERS)
	$(CC) $(CFLAGS) -I. $(MAIN) -o main
run: main
	./main
clean:
	$(RM) main guest cleaner
.PHONY: run