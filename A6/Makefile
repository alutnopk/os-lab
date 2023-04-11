CC = g++
CFLAGS = -Wall -Wextra
RM = rm -f
AR = ar
TARGET_LIB = libgoodmalloc.a
PROGRAM = mergesort.cpp

a.out: $(TARGET_LIB) $(PROGRAM)
	$(CC) $(CFLAGS) $(PROGRAM) -L. -lgoodmalloc

$(TARGET_LIB): goodmalloc.o
	$(AR) rcs $@ $^

goodmalloc.o: goodmalloc.hpp goodmalloc.cpp
	$(CC) $(CFLAGS) -c goodmalloc.cpp

clean:
	$(RM) goodmalloc.o libgoodmalloc.a a.out