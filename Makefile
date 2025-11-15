SOURCE_FILES = $(shell find src -maxdepth 1 -name "*.c")
BINARY_FILES = $(shell find src/bin -maxdepth 1 -name "*.c")
OBJECT_TARGETS = $(patsubst src/%.c,build/%.o,$(SOURCE_FILES))
BINARY_TARGETS = $(patsubst src/bin/%.c,bin/%,$(BINARY_FILES))

.PHONY: all bin clean
.SECONDARY: $(OBJECT_TARGETS)

build/%.o: src/%.c
	mkdir -p $(dir $@)
	gcc -Wall -Wextra -c $< -o $@

bin/%: src/bin/%.c $(OBJECT_TARGETS)
	mkdir -p $(dir $@)
	gcc -o $@ $< $(OBJECT_TARGETS)

bin: $(BINARY_TARGETS)

clean:
	rm -rf build
	rm -rf lib
	rm -rf bin

all: bin

