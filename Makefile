CC      = clang
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude
AR      = ar
ARFLAGS = rcs
BUILD   = build

all: $(BUILD)/libhobo.a

$(BUILD)/libhobo.a: $(BUILD)/arena.o
	$(AR) $(ARFLAGS) $@ $^

$(BUILD)/arena.o: src/arena.c include/hobo/arena.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(BUILD)/test_arena
	./$(BUILD)/test_arena

$(BUILD)/test_arena: tests/test_arena.c $(BUILD)/libhobo.a | $(BUILD)
	$(CC) $(CFLAGS) -fsanitize=address,undefined $< -L$(BUILD) -lhobo -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
