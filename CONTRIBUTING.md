# Contributing to QAccelPlot

Thank you for your interest in QAccelPlot. Feedback from real-world use helps
shape the library, particularly while the 0.x releases are still evolving.

## Issues

Please use [GitHub Issues](https://github.com/michalgrabarczyk/QAccelPlot/issues)
to report bugs, request features, describe compatibility problems, or discuss
possible improvements. Helpful reports can include:

- a clear description of the problem or proposed improvement;
- steps to reproduce the problem;
- relevant platform, compiler, Qt, and graphics-backend information; and
- logs, screenshots, or minimal test data.

## Pull requests

Pull requests are welcome. For larger changes or new API, open an issue first
so the design can be agreed before you write the code.

Before opening a pull request:

- follow the conventions in [`AGENTS.md`](AGENTS.md);
- format changed `.cpp` and `.hpp` files with clang-format 19;
- build with `-DQACCELPLOT_BUILD_TESTS=ON` and run
  `ctest --test-dir build --output-on-failure`;
- add tests for fixed bugs and new behavior; and
- add a `CHANGELOG.md` entry under `[Unreleased]` for user-visible changes.

New source files start with the same SPDX header as existing files.

## License

QAccelPlot is licensed under the [MIT License](LICENSE). By submitting a
contribution, you agree that it is licensed under the same terms.
