CC = g++
CFLAGS = -Wall -Wextra
RM = rm -f
HEADERS = headers.h
MAIN = main.cpp
GUEST = guest.cpp
CLEANER = cleaner.cpp

a.out: $(HEADERS) main.o guest.o cleaner.o
	$(CC) $(CFLAGS) main.o guest.o cleaner.o -o a.out
main.o: $(MAIN) $(HEADERS)
	$(CC) $(CFLAGS) -c $(MAIN) -o main.o
guest.o: $(GUEST) $(HEADERS)
	$(CC) $(CFLAGS) -c $(GUEST) -o guest.o
cleaner.o: $(CLEANER) $(HEADERS)
	$(CC) $(CFLAGS) -c $(CLEANER) -o cleaner.o
clean:
	$(RM) main.o guest.o cleaner.o a.out


