SOURCE_FILES = $(shell find src/lib -name "*.c")
BINARY_FILES = $(shell find src/bin -name "*.c")
OBJECT_TARGETS = $(patsubst src/lib/%.c,build/%.o,$(SOURCE_FILES))
BINARY_TARGETS = $(patsubst src/bin/%.c,bin/%,$(BINARY_FILES))

.PHONY: all bin clean

build/%.o: src/lib/%.c
	mkdir -p $(dir $@)
	gcc -Wall -Wextra -c $< -o $@

bin/%: src/bin/%.c lib/libchess.a
	mkdir -p $(dir $@)
	gcc -o $@ $< -Iinclude -Llib -lchess

lib/libchess.a: $(OBJECT_TARGETS)
	mkdir -p lib
	ar rcs lib/libchess.a $(OBJECT_TARGETS)

bin: $(BINARY_TARGETS)

all: lib/libchess.a bin
	@echo $(BINARY_TARGETS)

clean:
	rm -rf build
	rm -rf lib
	rm -rf bin
