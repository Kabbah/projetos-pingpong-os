TARGET0 = contexts
TARGET1 = pingpong-tasks1
TARGET2 = pingpong-tasks2
TARGET3 = pingpong-tasks3
LIBS = 
CC = gcc
CFLAGS = -Wall -Wextra

.PHONY: default all clean

default: $(TARGET0) $(TARGET1) $(TARGET2) $(TARGET3)
all: default
debug: default

OBJECTS = pingpong.o
OBJECT0 = contexts.o
OBJECT1 = pingpong-tasks1.o
OBJECT2 = pingpong-tasks2.o
OBJECT3 = pingpong-tasks3.o
HEADERS = $(wildcard *.h)

debug: DEBUG = -DDEBUG

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET0): $(OBJECTS) $(OBJECT0)
	$(CC) $(OBJECTS) $(OBJECT0) $(CFLAGS) $(LIBS) -o $@

$(TARGET1): $(OBJECTS) $(OBJECT1)
	$(CC) $(OBJECTS) $(OBJECT1) $(CFLAGS) $(LIBS) -o $@

$(TARGET2): $(OBJECTS) $(OBJECT2)
	$(CC) $(OBJECTS) $(OBJECT2) $(CFLAGS) $(LIBS) -o $@

$(TARGET3): $(OBJECTS) $(OBJECT3)
	$(CC) $(OBJECTS) $(OBJECT3) $(CFLAGS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET0) $(TARGET1) $(TARGET2) $(TARGET3) 
