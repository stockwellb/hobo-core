# C Concepts in hobo-core

This document is a guided tour of the C programming concepts used in this
library, written for someone learning C. **hobo** is a tiny unit-testing
framework. Reading its source is a good way to see real C idioms in a small
space.

The library has four pieces:

| File | What it does |
|------|--------------|
| `arena.c` / `arena.h` | A simple memory allocator (an "arena"). |
| `check.c` / `check.h` | Records the result of each `CHECK(...)` in a test. |
| `test.c` / `test.h` | Runs a suite of tests and tallies pass/fail/skip. |
| `tap_reporter.c` / `tap_reporter.h` | Prints results in the TAP format. |

The concepts are split into two groups:

1. **Low-level concepts** — pointers, memory, and how the machine works.
2. **Higher-level concepts** — abstractions and patterns built on top of those.

---

## Part 1 — Low-level concepts

These are the "close to the metal" features. They deal with raw memory
addresses, individual bytes, and the exact size and shape of data.

### 1.1 Pointers

A **pointer** is a variable that holds the *address* of another piece of data,
rather than the data itself. You declare one with `*`.

```c
unsigned char *base;   // a pointer to a byte somewhere in memory
```
*(`arena.h:8`)*

If `base` holds address `1000`, then `*base` ("dereference base") is the byte
*living at* address `1000`. Pointers are how C lets you share and modify data
without copying it.

Throughout the library, functions take a pointer to a struct so they can modify
the caller's object in place:

```c
bool hobo_arena_init(hobo_arena *arena, size_t capacity);
```
*(`arena.h:13`)* — the function writes into the arena you hand it, instead of
returning a fresh copy.

### 1.2 `void *` — the generic pointer

A `void *` is a pointer that has **no type attached**. It is just "an address",
and you can convert any other pointer to and from it. This is C's way of saying
"a pointer to *something*, I'm not saying what."

```c
void *hobo_arena_alloc(hobo_arena *arena, size_t size);
```
*(`arena.h:15`)* — the allocator returns raw memory; the caller decides what
type to treat it as.

The test framework uses `void *` to pass a test's "context" around without the
framework needing to know its real type:

```c
void (*setup)(void *ctx);
bool (*run)(void *ctx);
```
*(`test.h:30-31`)*

Inside a test, you convert that `void *` back to the real type:

```c
arena_ctx *ctx = raw;   // raw is a void *, assigned to a typed pointer
```
*(`test_arena.c:24`)* — no cast is needed when assigning *to or from* `void *`
in C.

### 1.3 Dereferencing and the address-of operator

- `&x` gives you "the address of `x`" — it produces a pointer.
- `*p` gives you "the value `p` points at" — it follows a pointer.
- `p->field` is shorthand for `(*p).field` — follow the pointer, then read a
  struct field.

```c
arena->base = malloc(capacity);   // arena is a pointer; -> reaches its field
```
*(`arena.c:6`)*

```c
return &sink;   // hand back the address of the file-global `sink`
```
*(`check.c:21`)*

### 1.4 Pointer arithmetic

When you add an integer to a pointer, C moves the pointer forward — but in units
of the pointed-to type, not raw bytes. Because `base` is an `unsigned char *`
(one byte per element), adding `aligned` moves forward exactly `aligned` bytes:

```c
void *ptr = arena->base + aligned;   // address `aligned` bytes into the arena
```
*(`arena.c:29`)*

The same idea drives **walking an array with a pointer**. Here `test++` steps to
the next element until it hits the sentinel (more on sentinels in 2.12):

```c
for (hobo_test_case *test = suite->tests; test->run != NULL; test++) {
  count++;
}
```
*(`test.c:17-19`)*

And `&sink.records[sink.count]` computes the address of the next free slot in an
array:

```c
hobo_check_record *slot = &sink.records[sink.count];
```
*(`check.c:28`)*

### 1.5 `malloc` and `free` — manual heap memory

C does not manage memory for you. When you need memory whose size is decided at
runtime, or that must outlive the function that created it, you ask the
operating system for it with `malloc` and give it back with `free`.

```c
arena->base = malloc(capacity);   // ask for `capacity` bytes
...
free(ctx->arena.base);            // give them back
```
*(`arena.c:6`, `test_arena.c:30`)*

Every `malloc` must eventually be matched by exactly one `free`. Forgetting to
free is a **memory leak**; freeing twice or using memory after freeing it is
**undefined behavior** (a bug that may crash or silently corrupt data).

### 1.6 `NULL` and checking for failure

`NULL` is a special pointer value meaning "points to nothing." C library
functions that can fail (like `malloc`) signal failure by returning `NULL`. You
must check for it:

```c
arena->base = malloc(capacity);
if (arena->base == NULL) {
  return false;          // allocation failed, report it upward
}
```
*(`arena.c:6-10`)*

The allocator itself returns `NULL` when there isn't enough room
(`arena.c:22`), and callers check that too (`test_arena.c:61-62`). Skipping
these checks is one of the most common sources of crashes in C.

### 1.7 Sized integer and size types: `size_t`, `uintptr_t`, `int`

C's plain `int` has no guaranteed size, and using a signed `int` for sizes and
counts can overflow or go negative. The standard library provides purpose-built
types:

- **`size_t`** — an unsigned integer big enough to hold the size of any object.
  Used everywhere for sizes, counts, and capacities.
  ```c
  size_t offset;
  size_t capacity;
  ```
  *(`arena.h:9-10`)*

- **`uintptr_t`** — an unsigned integer wide enough to hold a *pointer value*.
  This lets you do integer math on an address, e.g. to check alignment:
  ```c
  CHECK((uintptr_t)p % 64 == 0);   // is the address a multiple of 64?
  ```
  *(`test_arena.c:46`)*

Matching the right integer type to the job avoids whole classes of overflow and
signedness bugs.

### 1.8 Casts

A **cast** `(type)value` explicitly converts a value from one type to another.
The library casts a pointer to an integer so it can do arithmetic on the
address:

```c
(uintptr_t)p % alignof(max_align_t) == 0
```
*(`test_arena.c:55`)* — you cannot use `%` on a pointer directly, so it is first
turned into a `uintptr_t`.

### 1.9 `unsigned char` as a "raw byte"

`unsigned char` is exactly one byte and has no surprising sign behavior, so C
programmers use it to mean "raw memory, just bytes." The arena's backing store
is a block of bytes:

```c
unsigned char *base;
```
*(`arena.h:8`)* — treating the buffer as bytes is what makes the
byte-accurate pointer arithmetic in 1.4 correct.

### 1.10 Bitwise operators and alignment math

CPUs require (or strongly prefer) that data of a given size live at an address
that is a multiple of some number — its **alignment**. The arena rounds the
current offset *up* to the next aligned address using a classic bit trick:

```c
size_t aligned = (arena->offset + align - 1) & ~(align - 1);
```
*(`arena.c:19`)*

Breaking it down (this works only when `align` is a power of two):

- `align - 1` is a mask of the low bits (e.g. `64` → `0b0111111`).
- `~(align - 1)` flips it (`~` is bitwise NOT) into a mask that clears those low
  bits.
- Adding `align - 1` first, then `&`-ing (bitwise AND) with that mask, rounds
  *up* to the nearest multiple of `align`.

This is far faster than division and is the standard way to align addresses.

### 1.11 `alignof` and `max_align_t`

`alignof(T)` (from `<stdalign.h>`) tells you the required alignment of type `T`.
`max_align_t` is a type whose alignment is the *strictest* the platform needs,
so aligning to it is safe for **any** data:

```c
return hobo_arena_alloc_aligned(arena, size, alignof(max_align_t));
```
*(`arena.c:36`)* — the convenient `hobo_arena_alloc` aligns every allocation
conservatively so the returned memory is usable for any type.

### 1.12 Overflow-safe arithmetic

Because `size_t` is unsigned, subtracting a larger number from a smaller one
*wraps around* to a huge value instead of going negative. A naive bounds check
like `aligned + size > capacity` can itself overflow and give the wrong answer.
The arena avoids that by rearranging the math so it never adds two large
numbers:

```c
if (aligned > arena->capacity || size > arena->capacity - aligned) {
  return NULL;
}
```
*(`arena.c:21`)* — `capacity - aligned` is only computed *after* confirming
`aligned <= capacity`, so the subtraction can't wrap. This kind of careful
ordering is a hallmark of robust low-level C.

### 1.13 `sizeof` and the array-length idiom

`sizeof x` gives the size of `x` in bytes at compile time. A common idiom gets
the number of elements in an array by dividing the array's total size by the
size of one element:

```c
hobo_check_begin(records, sizeof records / sizeof records[0]);
```
*(`test.c:33`)* — `sizeof records` is the whole array's bytes; `sizeof
records[0]` is one element; the quotient is the element count. Writing it this
way means the count stays correct even if you change the array's size later.

---

## Part 2 — Higher-level concepts

These are the abstractions and design patterns C lets you build out of the
low-level pieces above. They are how a C codebase stays organized and reusable.

### 2.1 `struct` — grouping related data

A `struct` bundles several values into one named type. The arena is three
related fields treated as a single thing:

```c
typedef struct hobo_arena {
  unsigned char *base;   // start of the memory block
  size_t offset;         // how much is used so far
  size_t capacity;       // total size of the block
} hobo_arena;
```
*(`arena.h:7-11`)*

Structs are the backbone of organizing data in C. Almost every type in this
library is a struct.

### 2.2 `typedef` — naming a type

`typedef` creates an alias for a type so you don't have to write `struct
hobo_arena` every time — just `hobo_arena`. Notice the pattern above:
`typedef struct hobo_arena { ... } hobo_arena;` defines the struct *and* its
short alias in one go. Some structs in the library are anonymous and exist only
through their typedef:

```c
typedef struct {
  bool passed;
  const char *expr;
  ...
} hobo_check_record;
```
*(`check.h:6-13`)*

### 2.3 `enum` — a set of named constants

An `enum` defines a type whose values are a fixed list of named integers. It
makes code read in words instead of magic numbers:

```c
typedef enum {
  HOBO_TEST_PASS,
  HOBO_TEST_FAIL,
  HOBO_TEST_SKIP,
} hobo_test_kind;
```
*(`test.h:9-13`)* — a test result is exactly one of these three states, and the
reporter switches on them (`tap_reporter.c:27`).

### 2.4 `const` — promising not to modify

`const` marks data the function will only read, not change. It documents intent
and lets the compiler catch accidental writes. The check record stores strings
it will never modify:

```c
const char *expr;      // the text of the checked expression
const char *file;      // the source file name
```
*(`check.h:8`, `check.h:11`)*

`const char *` is the usual C type for "a string I'm borrowing and won't
change."

### 2.5 Function pointers

A **function pointer** is a variable that holds the address of a *function*, so
you can call different code through the same variable. The syntax
`return_type (*name)(params)` declares one. A test case stores pointers to the
functions that set it up, run it, and tear it down:

```c
typedef struct {
  const char *name;
  void (*setup)(void *ctx);
  bool (*run)(void *ctx);
  void (*teardown)(void *ctx);
  const char *skip;
} hobo_test_case;
```
*(`test.h:28-35`)*

The runner calls them through the pointers without knowing what they actually
do:

```c
if (test->setup != NULL) {
  test->setup(ctx);        // call whatever function this test plugged in
}
bool ok = test->run(ctx);
```
*(`test.c:46-50`)* — this is what makes the framework reusable for *any* test.

### 2.6 Function pointers as an interface (polymorphism)

Group several function pointers into a struct and you get something like an
*interface* or *virtual table* from object-oriented languages: a bundle of
operations that different implementations can fill in differently. The
**reporter** is exactly this:

```c
typedef struct hobo_reporter {
  void *state;
  void (*suite_begin)(void *state, const hobo_test_suite *suite, size_t test_count);
  void (*test_end)(void *state, const hobo_test_case *test, const hobo_test_result *result);
  void (*suite_end)(void *state, const hobo_test_summary *summary);
} hobo_reporter;
```
*(`test.h:44-51`)*

The test runner only knows about this interface; it calls
`reporter->test_end(...)` (`test.c:66`) without caring how results are
displayed. The TAP reporter is one implementation that fills the slots in with
functions that print TAP output:

```c
hobo_reporter reporter = {
    .state = &state,
    .suite_begin = tap_suite_begin,
    .test_end = tap_test_end,
    .suite_end = tap_suite_end,
};
```
*(`tap_reporter.c:55-60`)* — you could write a JSON reporter or a colored
console reporter and the runner wouldn't change at all. This is **polymorphism
in plain C**.

### 2.7 The opaque-context pattern (`void *state` / `void *ctx`)

Notice that the interface above carries a `void *state`, and each callback
receives it back. This lets each implementation stash its own private data
without the generic runner knowing its type. The TAP reporter's state is its
output stream and a counter:

```c
typedef struct {
  FILE *out;
  size_t test_number;
} tap_state;
```
*(`tap_reporter.c:6-9`)*

Inside the callback it converts the `void *` back to its real type:

```c
static void tap_test_end(void *st, ...) {
  tap_state *s = st;     // recover the concrete type
  ...
}
```
*(`tap_reporter.c:22-24`)* — the same trick the tests use with their
`arena_ctx`. This is how C writes generic, reusable code without templates or
generics.

### 2.8 Designated initializers

When building a struct, C lets you name the fields you're setting instead of
relying on order. This is clearer and less error-prone:

```c
hobo_test_summary summary = {
    .total = count,
    .passed = passed,
    .failed = failed,
    .skipped = skipped,
};
```
*(`test.c:73-78`)* — any field you don't mention is set to zero.

### 2.9 Returning a struct by value

Functions can return whole structs, not just pointers. `hobo_tap_reporter`
builds a reporter and hands the entire struct back to the caller:

```c
hobo_reporter hobo_tap_reporter(FILE *out) {
  ...
  hobo_reporter reporter = { ... };
  return reporter;        // the whole struct is copied out
}
```
*(`tap_reporter.c:53-61`)* — small structs are routinely passed and returned by
value in modern C.

### 2.10 The arena allocator pattern

The arena itself is a higher-level *design pattern* for managing memory. Instead
of many individual `malloc`/`free` calls, you `malloc` one big block up front and
then hand out pieces of it by simply bumping an offset forward
(`arena.c:30`). Allocation becomes nearly free, and you release everything at
once by freeing the single block.

The trade-off: you cannot free individual allocations — only the whole arena.
This is ideal for data with a shared lifetime (e.g. everything needed for one
test, one request, or one frame). Knowing *which* allocation strategy fits a
problem is a key skill in systems programming.

### 2.11 Separating interface from implementation (`.h` vs `.c`)

C splits code into **header files** (`.h`, the public interface) and **source
files** (`.c`, the private implementation). Other code `#include`s the header to
learn *what* functions exist, while the `.c` file holds *how* they work.

- `arena.h` declares `hobo_arena_alloc` (`arena.h:15`).
- `arena.c` defines what it actually does (`arena.c:35`).

This lets you compile pieces separately and hide implementation details. The
finished `.c` files are bundled into the static library `libhobo.a` that
programs link against.

### 2.12 Sentinel-terminated arrays

C arrays don't carry their own length. One classic solution is a **sentinel**: a
special end-marker element. The array of tests ends with an all-zero element
`{0}`, and the runner loops until it sees a test whose `run` pointer is `NULL`:

```c
static hobo_test_case arena_tests[] = {
    {"test_arena_init", test_setup, test_arena_init, test_teardown, 0},
    ...
    {0}};                 // <-- the sentinel
```
*(`test_arena.c:66-73`)*

```c
for (hobo_test_case *test = suite->tests; test->run != NULL; test++)
```
*(`test.c:17`)* — this is the same idea as C's NUL-terminated strings.

### 2.13 Header include guards

If two files both `#include` the same header, its contents could be pasted in
twice and cause "redefinition" errors. An **include guard** prevents that: the
first inclusion defines a macro, and any later one sees it's already defined and
skips the body.

```c
#ifndef HOBO_ARENA_H
#define HOBO_ARENA_H
...
#endif
```
*(`arena.h:1-2`, `arena.h:19`)* — every header in the library follows this
pattern.

### 2.14 Macros and the preprocessor

A **macro** is a textual substitution performed *before* compilation by the
preprocessor (`#define`). The library's headline feature, `CHECK`, is a macro:

```c
#define CHECK(cond) test_check(!!(cond), #cond, __FILE__, __LINE__)
```
*(`check.h:28`)*

Why a macro instead of a function? Because a macro can capture things a function
cannot, shown next.

### 2.15 Stringizing (`#`) and predefined macros (`__FILE__`, `__LINE__`)

Inside a macro, the `#` operator turns a parameter into a **string literal** of
its source text. So `CHECK(x == 1)` expands with `#cond` becoming the literal
`"x == 1"` — that's how a failed check can print the exact expression you wrote:

```c
#define CHECK(cond) test_check(!!(cond), #cond, __FILE__, __LINE__)
```
*(`check.h:28`)*

`__FILE__` and `__LINE__` are **predefined macros** the compiler fills in with
the current file name and line number. Together they let a failure report
*where* it happened:

```c
fprintf(s->out, "# %s - %s:%d\n", r->expr, r->file, r->line);
```
*(`tap_reporter.c:41`)* — printing the captured expression, file, and line.

### 2.16 The `!!` "normalize to 0 or 1" idiom

`!x` is logical NOT. Applying it twice, `!!x`, converts *any* nonzero value to
`1` and zero to `0` — a clean boolean. `CHECK` uses it so the recorded result is
always a tidy true/false regardless of what the condition evaluated to:

```c
test_check(!!(cond), ...)
```
*(`check.h:28`)*

### 2.17 `static` — internal linkage and private state

The `static` keyword at file scope means "private to this file." It is C's main
tool for *encapsulation*. It's used two ways here:

- **Private functions** the rest of the program shouldn't call directly:
  ```c
  static void tap_test_end(void *st, ...) { ... }
  ```
  *(`tap_reporter.c:22`)*

- **Private module state** — a single hidden variable shared by a file's
  functions. The check system keeps its results in one file-local `sink`:
  ```c
  static hobo_check_sink sink;
  ```
  *(`check.c:3`)* — code outside `check.c` can only reach it through the public
  `hobo_check_get()` accessor (`check.c:21`).

### 2.18 The callback / lifecycle pattern (setup → run → teardown)

Built on function pointers (2.5), the runner drives each test through a fixed
**lifecycle**: optional setup, the test body, optional teardown — plus
suite-wide setup/teardown around the whole batch. Each stage is optional and
guarded by a `NULL` check:

```c
if (test->setup != NULL) test->setup(ctx);
bool ok = test->run(ctx);
if (test->teardown != NULL) test->teardown(ctx);
```
*(`test.c:46-53`)*

This "template" of fixed steps with pluggable behavior is a widely reused design
pattern (you'll see it in many test frameworks and game/UI loops).

### 2.19 Suppressing "unused parameter" warnings with `(void)`

An interface (2.6) forces every implementation to accept the same parameters,
even ones a particular implementation ignores. Casting an argument to `void`
tells the compiler "I'm deliberately not using this," silencing the warning:

```c
static void tap_suite_end(void *st, const hobo_test_summary *summary) {
  (void)st;
  (void)summary;
}
```
*(`tap_reporter.c:48-51`)* — a small but common bit of professional C hygiene.

### 2.20 Standard I/O: `FILE *` and formatted output

C's `<stdio.h>` represents an open output destination (a file, the terminal,
etc.) as a `FILE *`. The reporter is handed one and writes to it, so the same
code can target the screen or a file:

```c
hobo_reporter reporter = hobo_tap_reporter(stdout);
```
*(`test_arena.c:83`)* — `stdout` is the standard output stream.

`fprintf` writes formatted text, using `%`-placeholders that are filled by the
later arguments. `%s` is a string, `%d` an `int`, and `%zu` a `size_t`:

```c
fprintf(s->out, "ok %zu - %s\n", n, test->name);
```
*(`tap_reporter.c:29`)* — matching each format specifier to the right argument
type is essential; a mismatch is undefined behavior.

### 2.21 Returning a status code from `main`

By convention a C program returns `0` for success and nonzero for failure. The
test program returns whether anything failed, so the shell (and CI systems) can
tell if the tests passed:

```c
int main(void) {
  hobo_reporter reporter = hobo_tap_reporter(stdout);
  return hobo_test_run_suite(&arena_suite, &reporter);
}
```
*(`test_arena.c:82-85`)* — and `hobo_test_run_suite` returns `failed != 0`
(`test.c:81`), i.e. `0` only when every test passed.

---

## How it all fits together

Following one `CHECK` from start to finish ties the concepts together:

1. You write `CHECK(p != NULL)` in a test. The **macro** (2.14) stringizes the
   expression and grabs the file/line (2.15), then calls `test_check`.
2. `test_check` writes a record into the file-local **`static` sink** (2.17)
   using **pointer arithmetic** into an array (1.4).
3. The **test runner** (2.18) calls your test through a **function pointer**
   (2.5), then asks the sink whether anything failed.
4. It packages a result struct (2.1, 2.8) and passes it to the **reporter
   interface** (2.6), which doesn't know or care that it's TAP.
5. The TAP reporter recovers its **opaque state** (2.7) and **`fprintf`s** (2.20)
   the outcome, including the captured expression, file, and line.
6. `main` returns the **status code** (2.21) so the outside world knows whether
   the suite passed.

That path — from a one-line macro down to raw memory and back up to a pluggable
reporter — is a compact tour of how C builds large, flexible systems out of
small, explicit parts.
