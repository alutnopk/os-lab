CFLAGS = -Wall -Wextra
PROGRAM_NAME = ass2

all: program

program: $(PROGRAM_NAME).c
	gcc $(CFLAGS) $(PROGRAM_NAME).c -o program

run: program
	./program
	
clean:
	rm -f program

.PHONY: all run clean
