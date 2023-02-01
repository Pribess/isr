CC = cc

CFLAGS = -Wall
LDFLAGS =

TARGET = isr

INCLUDEDIR = 
SOURCES = $(shell find ./src -name "*.c")
OBJECTS = $(SOURCES:.c=.o)


$(TARGET) : $(OBJECTS)
	$(CC) $(notdir $^) $(LDFLAGS) -lm -o $@

%.o : %.c
	$(CC) $(INCLUDEDIR) -c $(CFLAGS) $< -o $(notdir $@)


.PHONY: all debug clean

all : $(TARGET)

debug: all
debug: CFLAGS += -g

clean:
	rm -f $(TARGET) *.o
