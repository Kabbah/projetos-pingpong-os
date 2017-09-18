TARGET0 = pingpong-preempcao
TARGET1 = signal
TARGET2 = timer
LIBS = 
CC = gcc
CFLAGS = -Wall

.PHONY: default all clean

default: $(TARGET0) $(TARGET1) $(TARGET2)
all: default
debug: default

OBJECTS = queue.o pingpong.o
OBJECT0 = pingpong-preempcao.o
OBJECT1 = signal.o
OBJECT2 = timer.o
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

clean:
	-rm -f *.o
	-rm -f $(TARGET0) $(TARGET1) $(TARGET2) 
