TARGET = pingpong-disco
LIBS = -lrt
CC = gcc
CFLAGS = -Wall

.PHONY: default all clean

default: $(TARGET)
all: default
debug: default

OBJECTS = queue.o harddisk.o pingpong.o
OBJECT = pingpong-disco.o
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

