CC = cc

INCLUDEDIR = -I$(PWD)/deps/jerryscript/install/include
CFLAGS = -Wall
LDFLAGS = -L$(PWD)/deps/jerryscript/install/lib -ljerry-core -ljerry-ext -ljerry-port-default -lm

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

jerry:
	cd deps/jerryscript && \
	tools/build.py --builddir=$(PWD)/deps/jerryscript/build --cmake-param="-DCMAKE_INSTALL_PREFIX=$(PWD)/deps/jerryscript/install/" && \
	$(MAKE) -C $(PWD)/deps/jerryscript/build install

js:
	cd src/script/js && $(MAKE) all

all : jerry js $(TARGET)

debug: all
debug: CFLAGS += -g

clean:
	rm -rf $(PWD)/deps/jerryscript/build
	rm -rf $(PWD)/deps/jerryscript/install
	rm -f $(TARGET) *.o
