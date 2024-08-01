CC = gcc
CFLAGS = -Wall -Wextra -g

OBJS = log.o list.o addr.o main.o

ahttpd: $(OBJS)
	$(CC) -o $@ $^

ev: event.o
	$(CC) -o ahttpd_ev $^ -levent

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o ahttpd ahttpd_ev
