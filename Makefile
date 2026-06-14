CC      = clang
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude
AR      = ar
ARFLAGS = rcs
BUILD   = build

all: $(BUILD)/libhobo.a

$(BUILD)/libhobo.a: $(BUILD)/arena.o $(BUILD)/test.o $(BUILD)/check.o $(BUILD)/tap_reporter.o
	$(AR) $(ARFLAGS) $@ $^

$(BUILD)/arena.o: src/arena.c include/hobo/arena.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/test.o: src/test.c include/hobo/test.h include/hobo/check.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/check.o: src/check.c include/hobo/check.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/tap_reporter.o: src/tap_reporter.c include/hobo/tap_reporter.h include/hobo/test.h | $(BUILD)
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
