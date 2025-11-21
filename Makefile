CC = gcc
WARNINGS = -Wall -Wextra -Wpedantic
CFLAGS =

SOURCE_FILES = $(shell find src src/gen -maxdepth 1 -name "*.c")
BINARY_FILES = $(shell find src/bin -maxdepth 1 -name "*.c")
OBJECT_TARGETS = $(patsubst src/%.c,build/%.o,$(SOURCE_FILES))
BINARY_TARGETS = $(patsubst src/bin/%.c,bin/%,$(BINARY_FILES))

.PHONY: all bin profile clean
.SECONDARY: $(OBJECT_TARGETS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARNINGS) -c $< -o $@

bin/%: src/bin/%.c $(OBJECT_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARNINGS) -o $@ $< $(OBJECT_TARGETS)

bin: $(BINARY_TARGETS)

clean:
	rm -rf build lib bin

all: bin

