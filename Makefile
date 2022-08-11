FFLIBS = glfw3 glew
CFLAGS = -g -ggdb -I/usr/local/include -Iinclude -Wall -Wextra $(shell pkg-config --cflags $(FFLIBS))

OS = $(shell uname)
ifeq ($(OS),Linux)
	GLFLAG = -lGL
else
	ifeq ($(OS),Darwin)
		GLFLAG = -framework OpenGL
	else
		GLFLAG = -lopengl32
	endif
endif

LFLAGS = $(shell pkg-config --libs $(FFLIBS)) -lm $(GLFLAG)
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
