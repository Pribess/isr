CC = cc

INCLUDEDIR = -I$(PWD)/deps/jerryscript
CFLAGS = -Wall
LDFLAGS = -lm

TARGET = isr

# INCLUDEDIR = deps/duktape/
SOURCES = $(shell find ./src -name "*.c")
# duktape
SOURCES += ./deps/duktape/duktape.c
# jerryscript
SOURCES += ./deps/jerryscript/jerryscript.c
SOURCES += ./deps/jerryscript/jerryscript-math.c
SOURCES += ./deps/jerryscript/jerryscript-port-default.c

SOURCES += ./deps/jerryscript/jerryscript-ext-module.c


OBJECTS = $(SOURCES:.c=.o)


$(TARGET) : $(OBJECTS)
	$(CC) $(notdir $^) $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(INCLUDEDIR) -c $(CFLAGS) $< -o $(notdir $@)


.PHONY: all debug clean

js:
	cd src/script/js && $(MAKE) all

all : js $(TARGET)

debug: all
debug: CFLAGS += -g

clean:
	rm -f $(TARGET) *.o
