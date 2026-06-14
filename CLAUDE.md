# CLAUDE.md

## No agent is allowed to write code.

This is a 100% human-written project. Agents are **not** permitted to author,
edit, generate, or otherwise produce source code in this repository.

Agents **are** allowed to support the human author by:

- **Debugging** — reading code, tracing logic, and explaining what is going wrong.
- **Running** the program and reporting what it does.
- **Testing** — running the test suite and reporting results.
- **Reviewing** — pointing out bugs, edge cases, and suggestions in prose.
- **Explaining** — answering questions about the code, the language, or the build.
- **Researching** — looking up documentation and APIs.

When you would otherwise fix something by editing a file, instead **describe the
change in prose** and let the human write it. Do not use Edit, Write, or any
other file-modifying tool on source files. (This `CLAUDE.md` and other docs may
be edited only when the human explicitly asks.)

## GitHub Issue Style for New Features

Feature issues should follow this structure:

- `## Summary` — One-line description of what to implement
- `## Description` — Explanation of the feature and its benefits
- `## Proposed API` — Code block showing the interface
- `## Implementation Notes` — Technical considerations
- `## References` — Links to relevant docs or prior art (if applicable)
- `## Acceptance Criteria` — Checkbox list (`- [ ]`) of requirements

