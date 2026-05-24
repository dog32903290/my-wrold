import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
FLOW = REPO_ROOT / "scripts" / "myworld_flow.py"


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def run_flow(*args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        [sys.executable, str(FLOW), *args],
        cwd=REPO_ROOT,
        text=True,
        capture_output=True,
    )
    if check and result.returncode != 0:
        raise AssertionError(
            f"flow command failed with {result.returncode}\nSTDOUT:\n{result.stdout}\nSTDERR:\n{result.stderr}"
        )
    return result


class MyWorldFlowTests(unittest.TestCase):
    def make_workspace(self, root: Path) -> None:
        write_text(
            root / "docs/superpowers/plans/2026-05-24-native-canvas-master-progress.md",
            "# Native Canvas Master Progress Plan\n\nCurrent active lane:\n\n```text\nC5 module publish/reuse path\n```\n",
        )

    def write_lock(self, root: Path, lane: str, owner: str, slice_name: str, status: str = "active") -> None:
        write_text(
            root / ".myworld/flow/locks" / f"{lane}.json",
            json.dumps(
                {
                    "lane": lane,
                    "slice": slice_name,
                    "owner": owner,
                    "status": status,
                    "updatedAt": "2026-05-24T12:00:00+08:00",
                },
                indent=2,
            )
            + "\n",
        )

    def test_lane_collision_handoff_prompt_launch_hook_and_next_claim(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            workspace = Path(directory)
            self.make_workspace(workspace)
            self.write_lock(workspace, "C5", "other-session", "C5.1")

            blocked = run_flow(
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "C5",
                "--slice",
                "C5.1",
                "--owner",
                "flow-test",
                check=False,
            )
            self.assertNotEqual(blocked.returncode, 0)
            self.assertIn("held by other-session", blocked.stderr)

            run_flow(
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "FLOWX",
                "--slice",
                "FLOWX.1",
                "--owner",
                "flow-test",
            )

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

            result = run_flow(
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
            )
            report = json.loads(result.stdout)
            self.assertTrue(report["ok"])
            self.assertEqual(report["handoffStatus"], "handoff-ready")
            self.assertTrue(proof_marker.exists())

            handoff = workspace / "docs/superpowers/handoffs/flow/current-handoff.md"
            boot_prompt = workspace / "docs/superpowers/handoffs/flow/next-session-boot-prompt.md"
            self.assertIn("FLOWX.1", handoff.read_text(encoding="utf-8"))
            self.assertIn("FLOWX.2", handoff.read_text(encoding="utf-8"))
            self.assertIn("FLOWX.2", boot_prompt.read_text(encoding="utf-8"))

            launch = json.loads(launch_marker.read_text(encoding="utf-8"))
            self.assertEqual(launch["lane"], "FLOWX")
            self.assertEqual(launch["nextSlice"], "FLOWX.2")
            self.assertEqual(Path(launch["bootPrompt"]).resolve(), boot_prompt.resolve())

            next_claim = run_flow(
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "FLOWX",
                "--slice",
                "FLOWX.2",
                "--owner",
                "next-session",
            )
            self.assertIn("claimed", next_claim.stdout)

    def test_failed_proof_does_not_emit_next_session_prompt(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            workspace = Path(directory)
            self.make_workspace(workspace)
            run_flow(
                "claim",
                "--workspace",
                str(workspace),
                "--lane",
                "FAILX",
                "--slice",
                "FAILX.1",
                "--owner",
                "flow-test",
            )

            failing_script = workspace / "fail.py"
            write_text(failing_script, "raise SystemExit(7)\n")

            failed = run_flow(
                "run-slice",
                "--workspace",
                str(workspace),
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
                check=False,
            )
            self.assertEqual(failed.returncode, 7)
            self.assertIn("proof failed", failed.stderr)
            self.assertFalse((workspace / "docs/superpowers/handoffs/flow/next-session-boot-prompt.md").exists())

            lock = json.loads((workspace / ".myworld/flow/locks/FAILX.json").read_text(encoding="utf-8"))
            self.assertEqual(lock["status"], "active")
            self.assertEqual(lock["slice"], "FAILX.1")

    def test_advance_lane_starts_named_c7_session_folder_and_launch_hook(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            workspace = Path(directory)
            self.make_workspace(workspace)

            proof_script = workspace / "c6-proof.py"
            proof_marker = workspace / "c6-proof-marker.txt"
            write_text(
                proof_script,
                "from pathlib import Path\n"
                f"Path({str(proof_marker)!r}).write_text('c6-proof-ok', encoding='utf-8')\n",
            )

            launch_script = workspace / "launch-c7.py"
            launch_marker = workspace / "launch-c7-marker.json"
            write_text(
                launch_script,
                "import json, os\n"
                "from pathlib import Path\n"
                "payload = {\n"
                "    'lane': os.environ['MYWORLD_FLOW_LANE'],\n"
                "    'nextSlice': os.environ['MYWORLD_FLOW_NEXT_SLICE'],\n"
                "    'sessionFolder': os.environ['MYWORLD_FLOW_SESSION_FOLDER'],\n"
                "    'bootPrompt': os.environ['MYWORLD_FLOW_BOOT_PROMPT'],\n"
                "}\n"
                f"Path({str(launch_marker)!r}).write_text(json.dumps(payload, indent=2), encoding='utf-8')\n",
            )

            result = run_flow(
                "advance-lane",
                "--workspace",
                str(workspace),
                "--from-lane",
                "C6",
                "--from-slice",
                "C6.2",
                "--next-lane",
                "C7",
                "--next-slice",
                "C7.1",
                "--owner",
                "hourly-flow-monitor",
                "--folder-name",
                "C7",
                "--proof-command",
                f"{sys.executable} {proof_script}",
                "--next-session-command",
                f"{sys.executable} {launch_script}",
            )
            report = json.loads(result.stdout)
            session_folder = workspace / ".myworld/flow/sessions/C7"
            boot_prompt = session_folder / "boot-prompt.md"
            session_json = session_folder / "session.json"

            self.assertTrue(report["ok"])
            self.assertEqual(report["nextLane"], "C7")
            self.assertEqual(Path(report["sessionFolder"]).resolve(), session_folder.resolve())
            self.assertTrue(proof_marker.exists())
            self.assertTrue(session_json.exists())
            self.assertTrue(boot_prompt.exists())
            self.assertIn("C7.1", boot_prompt.read_text(encoding="utf-8"))

            session = json.loads(session_json.read_text(encoding="utf-8"))
            self.assertEqual(session["lane"], "C7")
            self.assertEqual(session["slice"], "C7.1")
            self.assertEqual(session["folderName"], "C7")
            self.assertEqual(session["previousLane"], "C6")
            self.assertEqual(session["previousSlice"], "C6.2")

            lock = json.loads((workspace / ".myworld/flow/locks/C7.json").read_text(encoding="utf-8"))
            self.assertEqual(lock["lane"], "C7")
            self.assertEqual(lock["slice"], "C7.1")
            self.assertEqual(lock["owner"], "hourly-flow-monitor")
            self.assertEqual(lock["status"], "active")
            self.assertEqual(Path(lock["sessionFolder"]).resolve(), session_folder.resolve())

            launch = json.loads(launch_marker.read_text(encoding="utf-8"))
            self.assertEqual(launch["lane"], "C7")
            self.assertEqual(launch["nextSlice"], "C7.1")
            self.assertEqual(Path(launch["sessionFolder"]).resolve(), session_folder.resolve())
            self.assertEqual(Path(launch["bootPrompt"]).resolve(), boot_prompt.resolve())

    def test_advance_lane_failed_proof_does_not_start_c7_session(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            workspace = Path(directory)
            self.make_workspace(workspace)
            failing_script = workspace / "fail-c6-proof.py"
            write_text(failing_script, "raise SystemExit(7)\n")

            failed = run_flow(
                "advance-lane",
                "--workspace",
                str(workspace),
                "--from-lane",
                "C6",
                "--from-slice",
                "C6.2",
                "--next-lane",
                "C7",
                "--next-slice",
                "C7.1",
                "--owner",
                "hourly-flow-monitor",
                "--folder-name",
                "C7",
                "--proof-command",
                f"{sys.executable} {failing_script}",
                check=False,
            )

            self.assertEqual(failed.returncode, 7)
            self.assertIn("proof failed", failed.stderr)
            self.assertFalse((workspace / ".myworld/flow/sessions/C7").exists())
            self.assertFalse((workspace / ".myworld/flow/locks/C7.json").exists())


if __name__ == "__main__":
    unittest.main()
