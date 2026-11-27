<!--
SPDX-FileCopyrightText: 2026 Michal Grabarczyk
SPDX-License-Identifier: MIT
-->

You are running unattended in the nightly architecture GitHub Actions job. Nobody can answer
questions, so these rules replace the interactive parts of the skill. Your result is one pull
request that the maintainer reviews in the morning.

## Inputs

- The domain glossary is `CONTEXT.md` (this repo has no `GLOSSARY.md`). ADRs are in `docs/adr/`.
- Earlier nightly runs: run
  `gh pr list --label nightly-architecture --state all --limit 30 --json number,title,state,body`.
  Don't propose a candidate an earlier nightly PR already covered. A closed, unmerged PR was
  rejected: read its comments with `gh pr view <number> --comments` and don't re-propose it.
- Focus for this run (empty means infer it from the commit history): `{{FOCUS}}`

## Report

Follow steps 1 and 2 of the skill, with these changes:

- Write the HTML report to `.nightly-architecture/report.html`. Don't open it.
- Don't ask which candidate to explore.

## Implement instead of grilling

Skip the grilling loop. Implement the top recommendation yourself:

1. Take the strongest candidate that fits one reviewable pull request: roughly 600 changed lines
   or fewer, and no breaking change to the public C++ or QML API. If the top recommendation
   doesn't fit, take the next one and say why in the PR body.
2. Answer the grilling questions yourself, using the `codebase-design` vocabulary. Record the
   decisions you made, and the ones the maintainer should check, in the PR body.
3. Keep the domain model current as the skill describes, but in `CONTEXT.md`. Don't write new
   ADRs; propose one in the PR body if a decision deserves it.
4. Follow `AGENTS.md`: C++ conventions, clean code, `///` docs, writing style. Add a
   `CHANGELOG.md` entry only for user-visible changes.

## Verify

Qt is installed and `build/` is already configured with tests and examples on.

- `clang-format-19 -i` every `.cpp` and `.hpp` file you changed.
- `python3 scripts/check-source-text.py`
- `cmake --build build --parallel 4`
- `xvfb-run -a ctest --test-dir build --output-on-failure -L '^unit-test$'`

Fix every build error, warning, and test failure you caused. Add or update tests through the
deepened module's interface.

## Hand-off

Never commit, push, or create branches or pull requests. Never edit files under `.github/`.
A later workflow step commits your working tree and opens the pull request.

If it succeeds, write:

- `.nightly-architecture/pr-title.txt`: one line, imperative, at most 70 characters.
- `.nightly-architecture/pr-body.md`: use the `pr` skill. Cover the finding (problem, files,
  why it ranked first), what changed, the decisions you made for the maintainer to check, and
  the other candidates in one line each. No test plan section and no AI attribution.

If no candidate is worth a pull request, or you can't get the build and tests green, discard
your source changes with `git restore` and `git clean` (never touch `.nightly-architecture/` or
`build/`), and write `.nightly-architecture/skip.md` saying why. No pull request is opened then.
