CC = gcc
CFLAGS = -Wall -Werror -std=c99
LDFLAGS = -lreadline

all: shell

shell: shell.o
	$(CC) $(CFLAGS) -o shell shell.o $(LDFLAGS)

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o shell
