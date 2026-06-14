# Library Roadmap

## High Priority

High usage features.
  - Dynamic array - stretchy buffer / vector
  - Hash table - at minimum string→void*
  - String builder - append, format, etc.
  - Read entire file - slurp file into arena

## Medium Priority

Common features.
  - String view/slice - non-owning string references
  - Sized typedefs - u8, u32, i64, usize, etc.
  - Path utilities - join, dirname, extension
  - Logging - leveled output with file/line

## Lower Priority

Nice to have features.
  - Pool allocator - fixed-size block allocation
  - Platform detection macros - OS/compiler/arch
  - Defer macro - scope-based cleanup
  - Endian utilities - byte swapping
