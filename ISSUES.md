# Test Harness — Known Issues

### 1. Code under test isn't sanitized
`tests/test_arena.c` builds with `-fsanitize=address,undefined`, but `libhobo.a`
(containing `arena.o`) is built clean (`Makefile:12`). UBSan never sees the alignment
arithmetic in `arena.c` (e.g. `align == 0` makes `~(align-1)` UB).
**Fix:** build the lib sources with the same sanitizer flags for the test target.

### 2. No suite-setup failure path
If `arena_suite_setup` returns NULL (malloc fail), every test NULL-derefs after the
plan line — no `Bail out!`. `run_suite` doesn't guard `ctx`.


### 3. Harness bundled into the shipped library
`libhobo.a` links `test.o` + `check.o` next to `arena.o` (`Makefile:9`), so consumers
drag in the test framework and its static buffers.
**Fix:** separate `libhobo_test.a`.

