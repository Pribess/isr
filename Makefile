CC = cc

CFLAGS = -Wall
LDFLAGS = -lm

TARGET = isr

# INCLUDEDIR = deps/duktape/
SOURCES = $(shell find ./src -name "*.c")
# duktape
SOURCES += ./deps/duktape/duktape.c

OBJECTS = $(SOURCES:.c=.o)


$(TARGET) : $(OBJECTS)
	$(CC) $(notdir $^) $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(INCLUDEDIR) -c $(CFLAGS) $< -o $(notdir $@)


.PHONY: all debug clean

generate_js:
	cd src/script/js && $(MAKE) all

all : generate_js $(TARGET)

debug: all
debug: CFLAGS += -g

clean:
	rm -f $(TARGET) *.o
