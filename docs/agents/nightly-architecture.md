# Nightly architecture review

`.github/workflows/nightly-architecture.yml` runs every day at 03:00 UTC. Claude runs the
`improve-codebase-architecture` skill from [mattpocock/skills](https://github.com/mattpocock/skills),
implements the top finding, and opens a pull request labeled `nightly-architecture`. The rules
for the unattended run are in `.github/prompts/nightly-architecture.md`.

## Setup

| Secret | Required | Purpose |
| --- | --- | --- |
| `CLAUDE_CODE_OAUTH_TOKEN` or `ANTHROPIC_API_KEY` | One of them | Claude authentication. Create the OAuth token with `claude setup-token` to bill a Claude subscription. |
| `NIGHTLY_PR_TOKEN` | No | Fine-grained token with Contents and Pull requests write access on this repo. Without it the pull request is opened with `GITHUB_TOKEN`, and GitHub doesn't start CI for it. |

Also enable **Settings → Actions → General → Allow GitHub Actions to create and approve pull
requests** when `NIGHTLY_PR_TOKEN` is not set.

## How a run works

| Job | Token | What it does |
| --- | --- | --- |
| `gate` | read | Skips the run while a `nightly-architecture` pull request is open. |
| `review` | read | Builds the project, runs Claude, and uploads the HTML report, the PR text, and the change as the `nightly-architecture` artifact. |
| `open-pr` | write | Applies the change on `dev/nightly-architecture-<date>`, refuses changes under `.github/`, commits, and opens the pull request. |

Claude never holds a write token and can't commit, push, or edit `.github/`.

## Steering

- **Run now or aim it**: run the workflow manually with `focus` set to a module or pain point.
  `force` runs it even when a nightly pull request is open.
- **Reject a finding**: close the pull request with a comment saying why. Later runs read
  closed nightly pull requests and don't propose the same candidate.
- **Reject it for good**: record the decision as an ADR in `docs/adr/`.
