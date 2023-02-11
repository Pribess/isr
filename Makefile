CC = cc

INCLUDEDIR = -I$(PWD)/deps/jerryscript
CFLAGS = -Wall
LDFLAGS = -lm

TARGET = isr

SOURCES = $(shell find ./src -name "*.c")
# duktape
SOURCES += ./deps/duktape/duktape.c
# jerryscript
SOURCES += ./deps/jerryscript/jerryscript.c
SOURCES += ./deps/jerryscript/jerryscript-port.c
SOURCES += ./deps/jerryscript/jerryscript-ext-module.c


OBJECTS = $(SOURCES:.c=.o)

vpath %.c $(shell find ./src -type d) $(shell find ./deps -type d)

$(TARGET) : $(notdir $(OBJECTS))
	$(CC) $^ $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(INCLUDEDIR) -c $(CFLAGS) $< -o $@

.PHONY: all debug clean js

all : js $(TARGET)

js:
	cd src/script/js && $(MAKE) all

debug: all
debug: CFLAGS += -g

clean:
	rm -f $(TARGET) *.o
