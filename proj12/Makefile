TARGET = pingpong-mqueue
LIBS = -lm
CC = gcc
CFLAGS = -Wall

.PHONY: default all clean

default: $(TARGET)
all: default
debug: default

OBJECTS = queue.o pingpong.o
OBJECT = pingpong-mqueue.o
HEADERS = $(wildcard *.h)

debug: DEBUG = -DDEBUG

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS) $(OBJECT)
	$(CC) $(OBJECTS) $(OBJECT) $(CFLAGS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

