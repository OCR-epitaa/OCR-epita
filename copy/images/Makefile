CC ?= gcc
CPPFLAGS = $(shell sdl2-config --cflags)
CFLAGS = -Wall -Wextra -Wvla -Werror -std=c99 -pedantic -I..
LDLIBS = $(shell sdl2-config --libs)

SRCS = ../window.c bitmap.c filter.c pixels.c main.c
OBJS = $(SRCS:.c=.o)
TARGET = grayscale

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

display: ../window.o bitmap.o display.o

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) display display.o

.PHONY: all clean
