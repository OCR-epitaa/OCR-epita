SRCS = xnor_network.c
CC = gcc
CFLAGS = -Wall -Wextra
LDLIBS = -lm

all: $(SRCS)
	$(CC) $(CFLAGS) $< -o xnor $(LDLIBS)

clean:
	$(RM) xnor
