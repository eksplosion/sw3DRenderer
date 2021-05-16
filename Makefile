SRCS = $(wildcard *.c)

OBJS = $(SRCS:.c=.o)

CFLAGS = -march=native -O2 -ffast-math -g
LDFLAGS = -lSDL2main -lSDL2 -lm -lpng

PROG = engine

all: $(PROG)

$(PROG): $(OBJS)
	gcc -o $(PROG) $(OBJS) $(LDFLAGS)
