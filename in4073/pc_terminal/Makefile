CC=gcc
CFLAGS = -g -Wextra -Wall -Werror -lm
EXEC = ./pc-terminal

default:
	$(CC) $(CFLAGS) pc_terminal.c -o $(EXEC)

clean:
	rm $(EXEC)

run: default
	$(EXEC)

run-ble: default
	sudo python ble_pty.py &
	sleep 5
	sudo $(EXEC) /dev/pts/13
