#!/usr/bin/env python3
import argparse
import json
import os
import shlex
import subprocess
import sys
import tempfile
from datetime import datetime
from pathlib import Path


MASTER_PROGRESS = Path("docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md")
FLOW_HANDOFF_DIR = Path("docs/superpowers/handoffs/flow")
FLOW_LOCK_DIR = Path(".myworld/flow/locks")
FLOW_SESSION_DIR = Path(".myworld/flow/sessions")


def now_iso() -> str:
    return datetime.now().astimezone().isoformat(timespec="seconds")


def safe_name(value: str, label: str) -> str:
    if not value or "/" in value or "\\" in value or value in {".", ".."}:
        raise FlowError(f"invalid {label}: {value!r}")
    return value


class FlowError(Exception):
    pass


def workspace_path(value: str | None) -> Path:
    return Path(value or ".").resolve()


def lock_path(workspace: Path, lane: str) -> Path:
    return workspace / FLOW_LOCK_DIR / f"{safe_name(lane, 'lane')}.json"


def session_path(workspace: Path, folder_name: str) -> Path:
    return workspace / FLOW_SESSION_DIR / safe_name(folder_name, "folder name")


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def extract_active_lane(master_text: str) -> str:
    marker = "Current active lane:"
    index = master_text.find(marker)
    if index < 0:
        return ""
    tail = master_text[index + len(marker) :]
    in_fence = False
    for raw_line in tail.splitlines():
        line = raw_line.strip()
        if line.startswith("```"):
            in_fence = not in_fence
            continue
        if in_fence and line:
            return line
    return ""


def load_master_status(workspace: Path) -> dict:
    path = workspace / MASTER_PROGRESS
    if not path.exists():
        return {"path": str(path), "exists": False, "activeLane": ""}
    text = path.read_text(encoding="utf-8")
    return {"path": str(path), "exists": True, "activeLane": extract_active_lane(text)}


def list_locks(workspace: Path) -> list[dict]:
    directory = workspace / FLOW_LOCK_DIR
    if not directory.exists():
        return []
    locks = []
    for path in sorted(directory.glob("*.json")):
        try:
            item = read_json(path)
            item["path"] = str(path)
            locks.append(item)
        except (OSError, json.JSONDecodeError) as exc:
            locks.append({"path": str(path), "status": "invalid", "error": str(exc)})
    return locks


def claim_lane(workspace: Path, lane: str, slice_name: str, owner: str) -> dict:
    lane = safe_name(lane, "lane")
    safe_name(slice_name, "slice")
    safe_name(owner, "owner")
    path = lock_path(workspace, lane)
    existing = read_json(path) if path.exists() else None

    if existing:
        existing_owner = existing.get("owner", "")
        existing_slice = existing.get("slice", "")
        existing_status = existing.get("status", "")

        if existing_status == "handoff-ready" and existing_slice == slice_name:
            payload = {
                **existing,
                "owner": owner,
                "previousOwner": existing_owner,
                "status": "active",
                "claimedAt": now_iso(),
                "updatedAt": now_iso(),
            }
            write_json(path, payload)
            return payload

        if existing_owner != owner:
            raise FlowError(
                f"lane {lane} is held by {existing_owner} at {existing_slice} ({existing_status})"
            )

        if existing_slice != slice_name:
            raise FlowError(
                f"lane {lane} is already held by {owner} at {existing_slice}; requested {slice_name}"
            )

    payload = {
        "lane": lane,
        "slice": slice_name,
        "owner": owner,
        "status": "active",
        "claimedAt": now_iso(),
        "updatedAt": now_iso(),
    }
    write_json(path, payload)
    return payload


def require_active_lock(workspace: Path, lane: str, slice_name: str, owner: str) -> dict:
    path = lock_path(workspace, lane)
    if not path.exists():
        raise FlowError(f"lane {lane} is not claimed")
    lock = read_json(path)
    if lock.get("owner") != owner:
        raise FlowError(f"lane {lane} is held by {lock.get('owner', '')}, not {owner}")
    if lock.get("slice") != slice_name:
        raise FlowError(f"lane {lane} lock is at {lock.get('slice', '')}, not {slice_name}")
    if lock.get("status") != "active":
        raise FlowError(f"lane {lane} lock is {lock.get('status', '')}, not active")
    return lock


def run_command(command: str, workspace: Path, env_extra: dict[str, str] | None = None) -> subprocess.CompletedProcess[str]:
    if not command.strip():
        raise FlowError("empty command")
    env = os.environ.copy()
    if env_extra:
        env.update(env_extra)
    return subprocess.run(
        shlex.split(command),
        cwd=workspace,
        text=True,
        capture_output=True,
        env=env,
    )


def handoff_markdown(
    *,
    workspace: Path,
    lane: str,
    completed_slice: str,
    next_slice: str,
    owner: str,
    proof_command: str,
    proof_stdout: str,
    proof_stderr: str,
    boot_prompt_path: Path,
) -> str:
    return (
        f"# Flow Handoff: {lane}\n\n"
        f"Updated: {now_iso()}\n\n"
        "## Current Line\n\n"
        f"- Lane: `{lane}`\n"
        f"- Completed slice: `{completed_slice}`\n"
        f"- Next slice: `{next_slice}`\n"
        f"- Previous owner: `{owner}`\n"
        f"- Workspace: `{workspace}`\n"
        f"- Master progress: `{workspace / MASTER_PROGRESS}`\n"
        f"- Lock: `{lock_path(workspace, lane)}`\n"
        f"- Boot prompt: `{boot_prompt_path}`\n\n"
        "## Proof\n\n"
        f"- Command: `{proof_command}`\n"
        "- Exit code: `0`\n\n"
        "## Proof Stdout\n\n"
        "```text\n"
        f"{proof_stdout.strip()}\n"
        "```\n\n"
        "## Proof Stderr\n\n"
        "```text\n"
        f"{proof_stderr.strip()}\n"
        "```\n\n"
        "## Next Session Rule\n\n"
        "Open the master progress plan first, then this handoff. Claim the lane at the next slice before editing files.\n"
    )


def boot_prompt_markdown(*, lane: str, next_slice: str, handoff_path: Path, workspace: Path) -> str:
    return (
        "# Next Flow Session Boot Prompt\n\n"
        "You are continuing an automated flow handoff for `我的世界`.\n\n"
        "Do this first:\n\n"
        f"1. Open `{workspace / MASTER_PROGRESS}`.\n"
        f"2. Open `{handoff_path}`.\n"
        f"3. Claim lane `{lane}` at slice `{next_slice}` with a new session owner.\n"
        "4. Do not edit any lane held by another owner.\n"
        "5. Run the slice proof before marking the slice complete.\n\n"
        f"Current next slice: `{next_slice}`\n"
    )


def run_slice(
    *,
    workspace: Path,
    lane: str,
    slice_name: str,
    owner: str,
    next_slice: str,
    proof_command: str,
    next_session_command: str,
) -> dict:
    require_active_lock(workspace, lane, slice_name, owner)
    proof = run_command(proof_command, workspace)
    if proof.returncode != 0:
        raise ProofFailed(proof.returncode, proof.stdout, proof.stderr)

    handoff_dir = workspace / FLOW_HANDOFF_DIR
    handoff_path = handoff_dir / "current-handoff.md"
    boot_prompt_path = handoff_dir / "next-session-boot-prompt.md"
    write_text(
        handoff_path,
        handoff_markdown(
            workspace=workspace,
            lane=lane,
            completed_slice=slice_name,
            next_slice=next_slice,
            owner=owner,
            proof_command=proof_command,
            proof_stdout=proof.stdout,
            proof_stderr=proof.stderr,
            boot_prompt_path=boot_prompt_path,
        ),
    )
    write_text(
        boot_prompt_path,
        boot_prompt_markdown(lane=lane, next_slice=next_slice, handoff_path=handoff_path, workspace=workspace),
    )

    lock = read_json(lock_path(workspace, lane))
    lock.update(
        {
            "slice": next_slice,
            "previousSlice": slice_name,
            "owner": owner,
            "status": "handoff-ready",
            "proofCommand": proof_command,
            "proofExitCode": 0,
            "handoffPath": str(handoff_path),
            "bootPromptPath": str(boot_prompt_path),
            "updatedAt": now_iso(),
        }
    )
    write_json(lock_path(workspace, lane), lock)

    launch_exit_code = None
    if next_session_command:
        launch = run_command(
            next_session_command,
            workspace,
            {
                "MYWORLD_FLOW_WORKSPACE": str(workspace),
                "MYWORLD_FLOW_LANE": lane,
                "MYWORLD_FLOW_COMPLETED_SLICE": slice_name,
                "MYWORLD_FLOW_NEXT_SLICE": next_slice,
                "MYWORLD_FLOW_HANDOFF": str(handoff_path),
                "MYWORLD_FLOW_BOOT_PROMPT": str(boot_prompt_path),
            },
        )
        launch_exit_code = launch.returncode
        if launch.returncode != 0:
            raise LaunchFailed(launch.returncode, launch.stdout, launch.stderr)

    return {
        "ok": True,
        "lane": lane,
        "completedSlice": slice_name,
        "nextSlice": next_slice,
        "handoffStatus": "handoff-ready",
        "handoffPath": str(handoff_path),
        "bootPromptPath": str(boot_prompt_path),
        "proofExitCode": proof.returncode,
        "nextSessionCommandExitCode": launch_exit_code,
    }


def session_boot_prompt_markdown(
    *,
    lane: str,
    slice_name: str,
    previous_lane: str,
    previous_slice: str,
    workspace: Path,
    session_folder: Path,
) -> str:
    return (
        f"# {lane} Session Boot Prompt\n\n"
        f"Session folder: `{session_folder}`\n"
        f"Workspace: `{workspace}`\n\n"
        "Start here:\n\n"
        f"1. Open `{workspace / MASTER_PROGRESS}` first.\n"
        f"2. Confirm `{previous_lane}` / `{previous_slice}` has proof, commit, and push evidence before editing.\n"
        f"3. Continue lane `{lane}` at slice `{slice_name}`.\n"
        "4. Work one smallest plan step at a time.\n"
        "5. For every step: implement, verify, update docs/handoff if needed, commit, then push.\n"
        "6. Do not touch files owned by another dirty lane.\n"
    )


def start_session(
    *,
    workspace: Path,
    lane: str,
    slice_name: str,
    owner: str,
    folder_name: str,
    previous_lane: str,
    previous_slice: str,
    next_session_command: str,
) -> dict:
    session_folder = session_path(workspace, folder_name)
    boot_prompt_path = session_folder / "boot-prompt.md"
    session_json_path = session_folder / "session.json"

    claim = claim_lane(workspace, lane, slice_name, owner)
    session_payload = {
        "lane": lane,
        "slice": slice_name,
        "owner": owner,
        "folderName": folder_name,
        "status": "active",
        "previousLane": previous_lane,
        "previousSlice": previous_slice,
        "workspace": str(workspace),
        "sessionFolder": str(session_folder),
        "bootPromptPath": str(boot_prompt_path),
        "masterProgressPath": str(workspace / MASTER_PROGRESS),
        "createdAt": now_iso(),
        "updatedAt": now_iso(),
    }
    write_json(session_json_path, session_payload)
    write_text(
        boot_prompt_path,
        session_boot_prompt_markdown(
            lane=lane,
            slice_name=slice_name,
            previous_lane=previous_lane,
            previous_slice=previous_slice,
            workspace=workspace,
            session_folder=session_folder,
        ),
    )

    lock = read_json(lock_path(workspace, lane))
    lock.update(
        {
            "sessionFolder": str(session_folder),
            "bootPromptPath": str(boot_prompt_path),
            "previousLane": previous_lane,
            "previousSlice": previous_slice,
            "updatedAt": now_iso(),
        }
    )
    write_json(lock_path(workspace, lane), lock)

    launch_exit_code = None
    if next_session_command:
        launch = run_command(
            next_session_command,
            workspace,
            {
                "MYWORLD_FLOW_WORKSPACE": str(workspace),
                "MYWORLD_FLOW_LANE": lane,
                "MYWORLD_FLOW_NEXT_SLICE": slice_name,
                "MYWORLD_FLOW_SESSION_FOLDER": str(session_folder),
                "MYWORLD_FLOW_BOOT_PROMPT": str(boot_prompt_path),
                "MYWORLD_FLOW_PREVIOUS_LANE": previous_lane,
                "MYWORLD_FLOW_PREVIOUS_SLICE": previous_slice,
            },
        )
        launch_exit_code = launch.returncode
        if launch.returncode != 0:
            raise LaunchFailed(launch.returncode, launch.stdout, launch.stderr)

    return {
        "ok": True,
        "lane": lane,
        "slice": slice_name,
        "nextLane": lane,
        "nextSlice": slice_name,
        "owner": owner,
        "folderName": folder_name,
        "sessionFolder": str(session_folder),
        "sessionJsonPath": str(session_json_path),
        "bootPromptPath": str(boot_prompt_path),
        "lockPath": str(lock_path(workspace, lane)),
        "claimStatus": claim.get("status", ""),
        "nextSessionCommandExitCode": launch_exit_code,
    }


def advance_lane(
    *,
    workspace: Path,
    from_lane: str,
    from_slice: str,
    next_lane: str,
    next_slice: str,
    owner: str,
    folder_name: str,
    proof_command: str,
    next_session_command: str,
) -> dict:
    proof = run_command(proof_command, workspace)
    if proof.returncode != 0:
        raise ProofFailed(proof.returncode, proof.stdout, proof.stderr)

    handoff_dir = workspace / FLOW_HANDOFF_DIR
    handoff_path = handoff_dir / f"{safe_name(from_lane, 'from lane')}-to-{safe_name(next_lane, 'next lane')}.md"
    write_text(
        handoff_path,
        (
            f"# Flow Advance: {from_lane} -> {next_lane}\n\n"
            f"Updated: {now_iso()}\n\n"
            f"- Previous lane: `{from_lane}`\n"
            f"- Previous slice: `{from_slice}`\n"
            f"- Next lane: `{next_lane}`\n"
            f"- Next slice: `{next_slice}`\n"
            f"- Proof command: `{proof_command}`\n"
            "- Proof exit code: `0`\n\n"
            "## Proof Stdout\n\n"
            "```text\n"
            f"{proof.stdout.strip()}\n"
            "```\n\n"
            "## Proof Stderr\n\n"
            "```text\n"
            f"{proof.stderr.strip()}\n"
            "```\n"
        ),
    )

    previous_lock = lock_path(workspace, from_lane)
    if previous_lock.exists():
        lock = read_json(previous_lock)
        lock.update(
            {
                "status": "closed",
                "closedAt": now_iso(),
                "nextLane": next_lane,
                "nextSlice": next_slice,
                "advanceHandoffPath": str(handoff_path),
                "updatedAt": now_iso(),
            }
        )
        write_json(previous_lock, lock)

    session = start_session(
        workspace=workspace,
        lane=next_lane,
        slice_name=next_slice,
        owner=owner,
        folder_name=folder_name,
        previous_lane=from_lane,
        previous_slice=from_slice,
        next_session_command=next_session_command,
    )
    session.update(
        {
            "advancedFromLane": from_lane,
            "advancedFromSlice": from_slice,
            "handoffPath": str(handoff_path),
            "proofExitCode": proof.returncode,
        }
    )
    return session


class ProofFailed(Exception):
    def __init__(self, code: int, stdout: str, stderr: str) -> None:
        super().__init__(f"proof failed with exit {code}")
        self.code = code
        self.stdout = stdout
        self.stderr = stderr


class LaunchFailed(Exception):
    def __init__(self, code: int, stdout: str, stderr: str) -> None:
        super().__init__(f"launch hook failed with exit {code}")
        self.code = code
        self.stdout = stdout
        self.stderr = stderr


def command_status(args: argparse.Namespace) -> int:
    workspace = workspace_path(args.workspace)
    payload = {
        "workspace": str(workspace),
        "masterProgress": load_master_status(workspace),
        "locks": list_locks(workspace),
    }
    print(json.dumps(payload, indent=2, ensure_ascii=False))
    return 0


def command_claim(args: argparse.Namespace) -> int:
    workspace = workspace_path(args.workspace)
    try:
        payload = claim_lane(workspace, args.lane, args.slice, args.owner)
    except FlowError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(f"claimed {payload['lane']} {payload['slice']} by {payload['owner']}")
    return 0


def command_run_slice(args: argparse.Namespace) -> int:
    workspace = workspace_path(args.workspace)
    try:
        payload = run_slice(
            workspace=workspace,
            lane=args.lane,
            slice_name=args.slice,
            owner=args.owner,
            next_slice=args.next_slice,
            proof_command=args.proof_command,
            next_session_command=args.next_session_command or "",
        )
    except ProofFailed as exc:
        print(f"proof failed with exit {exc.code}", file=sys.stderr)
        if exc.stderr.strip():
            print(exc.stderr.strip(), file=sys.stderr)
        return exc.code
    except LaunchFailed as exc:
        print(f"launch hook failed with exit {exc.code}", file=sys.stderr)
        if exc.stderr.strip():
            print(exc.stderr.strip(), file=sys.stderr)
        return exc.code
    except FlowError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(json.dumps(payload, indent=2, ensure_ascii=False))
    return 0


def command_start_session(args: argparse.Namespace) -> int:
    workspace = workspace_path(args.workspace)
    try:
        payload = start_session(
            workspace=workspace,
            lane=args.lane,
            slice_name=args.slice,
            owner=args.owner,
            folder_name=args.folder_name or args.lane,
            previous_lane=args.previous_lane or "",
            previous_slice=args.previous_slice or "",
            next_session_command=args.next_session_command or "",
        )
    except LaunchFailed as exc:
        print(f"launch hook failed with exit {exc.code}", file=sys.stderr)
        if exc.stderr.strip():
            print(exc.stderr.strip(), file=sys.stderr)
        return exc.code
    except FlowError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(json.dumps(payload, indent=2, ensure_ascii=False))
    return 0


def command_advance_lane(args: argparse.Namespace) -> int:
    workspace = workspace_path(args.workspace)
    try:
        payload = advance_lane(
            workspace=workspace,
            from_lane=args.from_lane,
            from_slice=args.from_slice,
            next_lane=args.next_lane,
            next_slice=args.next_slice,
            owner=args.owner,
            folder_name=args.folder_name or args.next_lane,
            proof_command=args.proof_command,
            next_session_command=args.next_session_command or "",
        )
    except ProofFailed as exc:
        print(f"proof failed with exit {exc.code}", file=sys.stderr)
        if exc.stderr.strip():
            print(exc.stderr.strip(), file=sys.stderr)
        return exc.code
    except LaunchFailed as exc:
        print(f"launch hook failed with exit {exc.code}", file=sys.stderr)
        if exc.stderr.strip():
            print(exc.stderr.strip(), file=sys.stderr)
        return exc.code
    except FlowError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    print(json.dumps(payload, indent=2, ensure_ascii=False))
    return 0


def run_cli(args: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run([sys.executable, __file__, *args], text=True, capture_output=True)


def command_experiment(_args: argparse.Namespace) -> int:
    checks = []
    with tempfile.TemporaryDirectory(prefix="myworld-flow-experiment-") as directory:
        workspace = Path(directory)
        write_text(
            workspace / MASTER_PROGRESS,
            "# Native Canvas Master Progress Plan\n\nCurrent active lane:\n\n```text\nC5 module publish/reuse path\n```\n",
        )
        write_json(
            lock_path(workspace, "C5"),
            {
                "lane": "C5",
                "slice": "C5.1",
                "owner": "other-session",
                "status": "active",
                "updatedAt": now_iso(),
            },
        )

        blocked = run_cli(
            [
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "C5",
                "--slice",
                "C5.1",
                "--owner",
                "flow-test",
            ]
        )
        checks.append({"name": "refuses lane held by another owner", "ok": blocked.returncode != 0 and "held by other-session" in blocked.stderr})

        claim = run_cli(
            [
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "FLOWX",
                "--slice",
                "FLOWX.1",
                "--owner",
                "flow-test",
            ]
        )
        checks.append({"name": "claims free experiment lane", "ok": claim.returncode == 0})

        proof_script = workspace / "proof.py"
        proof_marker = workspace / "proof-marker.txt"
        write_text(
            proof_script,
            "from pathlib import Path\n"
            f"Path({str(proof_marker)!r}).write_text('proof-ok', encoding='utf-8')\n",
        )
        launch_script = workspace / "launch.py"
        launch_marker = workspace / "launch-marker.json"
        write_text(
            launch_script,
            "import json, os\n"
            "from pathlib import Path\n"
            "payload = {\n"
            "    'bootPrompt': os.environ['MYWORLD_FLOW_BOOT_PROMPT'],\n"
            "    'lane': os.environ['MYWORLD_FLOW_LANE'],\n"
            "    'nextSlice': os.environ['MYWORLD_FLOW_NEXT_SLICE'],\n"
            "}\n"
            f"Path({str(launch_marker)!r}).write_text(json.dumps(payload, indent=2), encoding='utf-8')\n",
        )
        run = run_cli(
            [
                "run-slice",
                "--workspace",
                str(workspace),
                "--lane",
                "FLOWX",
                "--slice",
                "FLOWX.1",
                "--owner",
                "flow-test",
                "--next-slice",
                "FLOWX.2",
                "--proof-command",
                f"{sys.executable} {proof_script}",
                "--next-session-command",
                f"{sys.executable} {launch_script}",
            ]
        )
        handoff_path = workspace / FLOW_HANDOFF_DIR / "current-handoff.md"
        boot_prompt_path = workspace / FLOW_HANDOFF_DIR / "next-session-boot-prompt.md"
        checks.append({"name": "runs proof and writes marker", "ok": run.returncode == 0 and proof_marker.exists()})
        checks.append({"name": "writes handoff and boot prompt", "ok": handoff_path.exists() and boot_prompt_path.exists()})
        checks.append({"name": "launch hook receives boot prompt", "ok": launch_marker.exists() and str(boot_prompt_path) in launch_marker.read_text(encoding="utf-8")})

        next_claim = run_cli(
            [
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "FLOWX",
                "--slice",
                "FLOWX.2",
                "--owner",
                "next-session",
            ]
        )
        checks.append({"name": "next session can claim handoff-ready slice", "ok": next_claim.returncode == 0})

        fail_workspace = workspace / "failed-proof"
        write_text(fail_workspace / MASTER_PROGRESS, "# Native Canvas Master Progress Plan\n")
        run_cli(
            [
                "claim",
                "--workspace",
                str(fail_workspace),
                "--lane",
                "FAILX",
                "--slice",
                "FAILX.1",
                "--owner",
                "flow-test",
            ]
        )
        failing_script = fail_workspace / "fail.py"
        write_text(failing_script, "raise SystemExit(7)\n")
        failed = run_cli(
            [
                "run-slice",
                "--workspace",
                str(fail_workspace),
                "--lane",
                "FAILX",
                "--slice",
                "FAILX.1",
                "--owner",
                "flow-test",
                "--next-slice",
                "FAILX.2",
                "--proof-command",
                f"{sys.executable} {failing_script}",
            ]
        )
        checks.append(
            {
                "name": "failed proof does not emit boot prompt",
                "ok": failed.returncode == 7
                and not (fail_workspace / FLOW_HANDOFF_DIR / "next-session-boot-prompt.md").exists(),
            }
        )

        advance_workspace = workspace / "advance"
        write_text(
            advance_workspace / MASTER_PROGRESS,
            "# Native Canvas Master Progress Plan\n\nCurrent active lane:\n\n```text\nNone after C6 closure.\n```\n",
        )
        advance_proof = advance_workspace / "c6-proof.py"
        advance_marker = advance_workspace / "c6-proof-marker.txt"
        write_text(
            advance_proof,
            "from pathlib import Path\n"
            f"Path({str(advance_marker)!r}).write_text('c6-proof-ok', encoding='utf-8')\n",
        )
        advance_launch = advance_workspace / "launch-c7.py"
        advance_launch_marker = advance_workspace / "launch-c7-marker.json"
        write_text(
            advance_launch,
            "import json, os\n"
            "from pathlib import Path\n"
            "payload = {\n"
            "    'lane': os.environ['MYWORLD_FLOW_LANE'],\n"
            "    'nextSlice': os.environ['MYWORLD_FLOW_NEXT_SLICE'],\n"
            "    'sessionFolder': os.environ['MYWORLD_FLOW_SESSION_FOLDER'],\n"
            "    'bootPrompt': os.environ['MYWORLD_FLOW_BOOT_PROMPT'],\n"
            "}\n"
            f"Path({str(advance_launch_marker)!r}).write_text(json.dumps(payload, indent=2), encoding='utf-8')\n",
        )
        advanced = run_cli(
            [
                "advance-lane",
                "--workspace",
                str(advance_workspace),
                "--from-lane",
                "C6",
                "--from-slice",
                "C6.2",
                "--next-lane",
                "C7",
                "--next-slice",
                "C7.1",
                "--owner",
                "flow-test",
                "--folder-name",
                "C7",
                "--proof-command",
                f"{sys.executable} {advance_proof}",
                "--next-session-command",
                f"{sys.executable} {advance_launch}",
            ]
        )
        checks.append({"name": "advance-lane proof runs", "ok": advanced.returncode == 0 and advance_marker.exists()})
        checks.append({"name": "advance-lane creates C7 session folder", "ok": (advance_workspace / FLOW_SESSION_DIR / "C7" / "session.json").exists()})
        checks.append({"name": "advance-lane launch hook sees C7 session", "ok": advance_launch_marker.exists() and "C7" in advance_launch_marker.read_text(encoding="utf-8")})

    ok = all(item["ok"] for item in checks)
    print(json.dumps({"ok": ok, "checks": checks}, indent=2, ensure_ascii=False))
    return 0 if ok else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Repo-local flow runner for 我的世界 session handoffs.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    status = subparsers.add_parser("status", help="Show master progress status and lane locks.")
    status.add_argument("--workspace", default=".")
    status.set_defaults(func=command_status)

    claim = subparsers.add_parser("claim", help="Claim a lane slice before editing.")
    claim.add_argument("--workspace", default=".")
    claim.add_argument("--lane", required=True)
    claim.add_argument("--slice", required=True)
    claim.add_argument("--owner", required=True)
    claim.set_defaults(func=command_claim)

    run = subparsers.add_parser("run-slice", help="Run a slice proof, write handoff, and trigger a launch hook.")
    run.add_argument("--workspace", default=".")
    run.add_argument("--lane", required=True)
    run.add_argument("--slice", required=True)
    run.add_argument("--owner", required=True)
    run.add_argument("--next-slice", required=True)
    run.add_argument("--proof-command", required=True)
    run.add_argument("--next-session-command", default="")
    run.set_defaults(func=command_run_slice)

    start = subparsers.add_parser("start-session", help="Create a named session folder, boot prompt, and lane lock.")
    start.add_argument("--workspace", default=".")
    start.add_argument("--lane", required=True)
    start.add_argument("--slice", required=True)
    start.add_argument("--owner", required=True)
    start.add_argument("--folder-name", default="")
    start.add_argument("--previous-lane", default="")
    start.add_argument("--previous-slice", default="")
    start.add_argument("--next-session-command", default="")
    start.set_defaults(func=command_start_session)

    advance = subparsers.add_parser("advance-lane", help="Verify a closed lane and start the next named session.")
    advance.add_argument("--workspace", default=".")
    advance.add_argument("--from-lane", required=True)
    advance.add_argument("--from-slice", required=True)
    advance.add_argument("--next-lane", required=True)
    advance.add_argument("--next-slice", required=True)
    advance.add_argument("--owner", required=True)
    advance.add_argument("--folder-name", default="")
    advance.add_argument("--proof-command", required=True)
    advance.add_argument("--next-session-command", default="")
    advance.set_defaults(func=command_advance_lane)

    experiment = subparsers.add_parser("experiment", help="Run a temp-workspace flow handoff experiment.")
    experiment.set_defaults(func=command_experiment)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
