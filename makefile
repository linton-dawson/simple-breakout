
CC=gcc
CFLAGS= -lepoxy -lpng -lm
DEPS= DashGL/dashgl.h
MAIN= main.c
OBDEP= DashGL/dashgl.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: DashGL/dashgl.h

DashGL/dashgl.h: $(OBDEP)
	$(CC) -c -o $@ $< $(CFLAGS)

	$(CC) `pkg-config --cflags gtk+-3.0` $(MAIN) $(OBDEP) `pkg-config --libs gtk+-3.0` $(CFLAGS)

	./a.out
