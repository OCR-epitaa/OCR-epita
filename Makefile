CC = gcc
CPPFLAGS = $(shell sdl2-config --cflags)
CFLAGS = -Wall -Wextra -Wvla -Werror -std=c99 -pedantic -Iinclude
LDLIBS = $(shell sdl2-config --libs)

SRCS = src/window.c src/button.c src/main.c src/ui.c
OBJS = $(SRCS:.c=.o)
TARGET = button

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

clean:
	$(RM) $(TARGET) $(OBJS)

.PHONY: all clean