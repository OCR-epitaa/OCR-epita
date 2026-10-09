CC ?= gcc
CPPFLAGS = $(shell sdl2-config --cflags)
CFLAGS = -Wall -Wextra -Wvla -Werror -std=c99 -pedantic -I..
LDLIBS = $(shell sdl2-config --libs)

SRCS = ../window.c calibration.c events.c main.c
OBJS = $(SRCS:.c=.o)
TARGET = calibration

all: $(TARGET)

$(TARGET): $(OBJS)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)

.PHONY: all clean
