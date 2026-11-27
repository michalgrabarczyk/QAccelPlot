# QAccelPlot Coding Standards

Judgement-call standards applied at review, alongside the conventions in
[`AGENTS.md`](AGENTS.md). Rules that CI checks (clang-format, clang-tidy,
source text, qmllint) are not repeated here.

## C++

- Keep functions short. When a function exceeds ~50 lines, extract smaller,
  well-named helper functions.
- Prefer `qreal` for geometric values exposed to QML; use `float` for
  performance-critical internal buffers.
- Comment only what the code can't say: an unintuitive change or the reason
  behind it. The `///` API documentation comments are always required.

## QML

- Use Qt Quick property bindings rather than imperative assignments where
  practical.

## Documentation

These rules apply to all documentation: the MkDocs guide, Doxygen comments,
README, CHANGELOG, and example descriptions.

- Be concise and precise. Say what something does, how to use it, and any
  constraints or pitfalls, then stop.
- Avoid filler prose: no introductions that restate the heading, no marketing
  language, no repeating what the code or API signature already shows.
- Prefer short sentences, bullet lists, tables, and code snippets over long
  paragraphs.
- Keep `\brief` descriptions to one sentence. Add details only when they
  convey behavior, units, ranges, threading, or ownership that the signature
  does not.
- Document only what exists. Don't describe planned features or speculate
  about future behavior.
- When updating docs, edit the existing text rather than appending new
  paragraphs that overlap with it.

## Examples

Each example describes itself with `ExampleHeader`:

- `title`: the example name.
- `subtitle`: one sentence saying what the example shows. Name the main type
  when the example is about it.
- `interactions`: at most four bullets, each one line at the default window
  size, saying what you can do. Start with a verb, use sentence case, and end
  without a period. Bold buttons, tabs, and keys with `<b>…</b>`. Name the tab
  when a bullet applies to one tab only. Leave out default pan, zoom, and reset
  unless they behave differently; leave out the list when there is nothing
  else to do.
