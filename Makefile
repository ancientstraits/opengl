OS = $(shell uname)
FFLIBS = glfw3 glew
CFLAGS = -g -ggdb -I/usr/local/include -Iinclude -Wall -Wextra $(shell pkg-config --cflags $(FFLIBS))
LFLAGS = $(shell pkg-config --libs $(FFLIBS)) -lm
ifeq ($(OS),Darwin)
	LFLAGS += -framework OpenGL
else
	LFLAGS += -lGL
endif

OBJS := $(patsubst  src/%.c, obj/%.o, $(wildcard src/*.c))
DEPS := $(wildcard include/*.h)
CC = cc
EXEC = main

all: $(EXEC)

obj:
	[ -d obj ] || mkdir obj

obj/%.o: src/%.c $(DEPS) | obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)
