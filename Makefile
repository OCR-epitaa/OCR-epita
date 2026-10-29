CC = gcc
CPPFLAGS = $(shell sdl2-config --cflags) $(shell pkg-config --cflags SDL2_ttf)
CFLAGS = -Wall -Wextra -Wvla -Werror -std=c99 -pedantic -Iinclude
LDLIBS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

SRCS = src/window.c src/button.c src/main.c src/ui.c src/ui_render.c src/bitmap.c src/Image_processing.c
OBJS = $(SRCS:.c=.o)
TARGET = button

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

clean:
	$(RM) $(TARGET) $(OBJS)

.PHONY: all clean