CC = cc

INCLUDEDIR = -I$(PWD)/deps/jerryscript
CFLAGS = -Wall
LDFLAGS = -lm

TARGET = isr

SOURCES = $(shell find ./src -name "*.c") $(shell find ./deps -name "*.c")
OBJECTS = $(SOURCES:.c=.o)

vpath %.c $(shell find ./src -type d) $(shell find ./deps -type d)

all : js $(TARGET)

$(TARGET) : $(notdir $(OBJECTS))
	$(CC) $^ $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(INCLUDEDIR) -c $(CFLAGS) $< -o $@

js:
	cd src/script/js && $(MAKE) all
	rm -f module.o

debug: all
debug: CFLAGS += -g

clean:
	rm -f $(TARGET) *.o

.PHONY: all debug clean js
