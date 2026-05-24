# Flow Runner Automation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a small repo-local flow runner that can claim a lane, run a proof command, write a handoff capsule, emit a next-session boot prompt, and call a replaceable launch hook.

**Architecture:** Keep chat sessions disposable and make repo artifacts carry continuity. The runner stores lane locks under `.myworld/flow/locks`, named session folders under `.myworld/flow/sessions`, writes handoff prompts under `docs/superpowers/handoffs/flow`, and refuses to take a lane held by another owner. The first experiment uses a temporary workspace so it can prove collision behavior without touching the active C5/C6 work owned by another session.

**Tech Stack:** Python 3 stdlib CLI, `unittest`, Markdown/JSON artifacts, shell proof commands.

---

## Contract

```text
master progress
-> lane lock
-> proof command
-> handoff capsule
-> next-session boot prompt
-> optional launch hook
```

The launch hook is intentionally a command boundary. Today it can be a fake session launcher in tests; later it can become a Codex app/automation bridge without changing the proof/handoff contract.

## Files

- Create: `scripts/myworld_flow.py`
  - CLI entrypoint.
  - Commands: `status`, `claim`, `run-slice`, `start-session`, `advance-lane`, `experiment`.
  - Owns lock JSON, handoff Markdown, boot prompt Markdown, proof command execution, and launch hook execution.
- Create: `tests/test_myworld_flow.py`
  - Python smoke tests that create a temporary workspace.
  - Proves lane collision refusal, successful FLOWX claim/run, handoff generation, boot prompt generation, C7 named session creation, failed-proof refusal, and launch hook invocation.
- Modify: `.gitignore`
  - Ignore root `.myworld/flow/` because locks/session folders are local runtime state, not source files.
- Modify: no C5 files.

## Acceptance

```text
python3 -m unittest tests/test_myworld_flow.py
python3 scripts/myworld_flow.py experiment
python3 scripts/myworld_flow.py status --workspace /Users/chenbaiwei/Desktop/我的世界
git diff --check -- scripts/myworld_flow.py tests/test_myworld_flow.py docs/superpowers/plans/2026-05-24-flow-runner-automation.md
```

## Tasks

### Task 1: Red Test For Flow Runner

- [x] Write `tests/test_myworld_flow.py` first.
- [x] Run `python3 -m unittest tests/test_myworld_flow.py`.
- [x] Expected red state: Python cannot open/import `scripts/myworld_flow.py`.

### Task 2: Minimal Runner

- [x] Create `scripts/myworld_flow.py`.
- [x] Implement `status`, `claim`, `run-slice`, and `experiment`.
- [x] Implement `start-session` and `advance-lane`.
- [x] Keep the lock format small:

```json
{
  "lane": "FLOWX",
  "slice": "FLOWX.1",
  "owner": "flow-test",
  "status": "active",
  "updatedAt": "2026-05-24T12:00:00+08:00"
}
```

### Task 3: Green Tests And Experiment

- [x] Run `python3 -B -m unittest tests/test_myworld_flow.py`.
- [x] Run `python3 -B scripts/myworld_flow.py experiment`.
- [x] Run `python3 -B scripts/myworld_flow.py status --workspace /Users/chenbaiwei/Desktop/我的世界`.
- [x] Run targeted whitespace check.

### Task 4: Risk Pressure

- [x] Confirm C5 collision is represented by a lock owned by another session in the experiment.
- [x] Confirm failed proof does not write a next-session prompt.
- [x] Confirm launch hook receives the generated boot prompt path.
- [x] Confirm C6 -> C7 advance creates `.myworld/flow/sessions/C7`.
- [x] Name remaining risks: stale locks, real Codex session launch adapter, proof command trust, and concurrent dirty files outside lock ownership.

## Experiment Result

Verified on 2026-05-24 with:

```text
python3 -B -m unittest tests/test_myworld_flow.py
python3 -B scripts/myworld_flow.py experiment
python3 -B scripts/myworld_flow.py status --workspace /Users/chenbaiwei/Desktop/我的世界
```

The temp-workspace experiment proves:

- a lane held by `other-session` is refused;
- a free `FLOWX` lane can be claimed;
- a proof command can complete and write evidence;
- `current-handoff.md` and `next-session-boot-prompt.md` are generated;
- the launch hook receives the boot prompt path through environment variables;
- a new owner can claim the handoff-ready next slice;
- a failed proof does not emit a next-session prompt.
- `advance-lane` runs a C6 proof, creates a named `C7` session folder, writes `session.json` and `boot-prompt.md`, claims a C7 lock, and passes the session folder to the launch hook.

Remaining risks:

- stale locks need an explicit human or future `release` command;
- this runner emits and launches a command hook, but does not yet call a real Codex session API;
- proof commands are trusted shell commands, so lane specs must keep them narrow and reviewable;
- dirty files outside `.myworld/flow/locks` still require `git status` discipline before edits.

## C6 To C7 Command Shape

When C6 proof/commit/push is truly complete, the next session can be started with:

```bash
python3 -B scripts/myworld_flow.py advance-lane \
  --workspace /Users/chenbaiwei/Desktop/我的世界 \
  --from-lane C6 \
  --from-slice C6.2 \
  --next-lane C7 \
  --next-slice C7.1 \
  --owner hourly-flow-monitor \
  --folder-name C7 \
  --proof-command "<C6 verification command>" \
  --next-session-command "<optional Codex launch adapter>"
```

This writes local runtime state under:

```text
.myworld/flow/sessions/C7/session.json
.myworld/flow/sessions/C7/boot-prompt.md
.myworld/flow/locks/C7.json
docs/superpowers/handoffs/flow/C6-to-C7.md
```
