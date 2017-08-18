CC = gcc
CFLAGS = -Wall

ODIR = obj
LDIR = 

LIBS = 

_DEPS = queue.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = testafila.o queue.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

testafila: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
