#!/usr/bin/env python3
"""Run one demo entry from examples/examples.json."""

from __future__ import annotations

import argparse
import json
import platform
import shlex
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MANIFEST_PATH = ROOT / "examples" / "examples.json"


def platform_name() -> str:
    sysname = platform.system().lower()
    if sysname == "darwin":
        return "mac"
    if sysname == "windows":
        return "windows"
    return "linux"


def load_manifest() -> dict:
    if not MANIFEST_PATH.exists():
        raise SystemExit(f"manifest not found: {MANIFEST_PATH}")
    return json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))


def resolve_command(command: str) -> str:
    return command.replace("${workspaceFolder}", str(ROOT))


def split_command(command: str) -> list[str]:
    if platform.system().lower() == "windows":
        parts = shlex.split(command, posix=False)
    else:
        parts = shlex.split(command)

    cleaned = []
    for part in parts:
        if len(part) >= 2 and (
            (part.startswith('"') and part.endswith('"')) or (part.startswith("'") and part.endswith("'"))
        ):
            cleaned.append(part[1:-1])
        else:
            cleaned.append(part)
    return cleaned


def all_entries(manifest: dict) -> list[dict]:
    entries = []
    gallery = manifest.get("gallery")
    if gallery:
        entries.append(gallery)
    entries.extend(manifest.get("demos", []))
    return entries


def find_entry(manifest: dict, selector: str) -> dict | None:
    if not selector:
        return None

    lower_selector = selector.lower()
    for entry in all_entries(manifest):
        if selector in (entry.get("id"), entry.get("target")):
            return entry
        if entry.get("name", "").lower() == lower_selector:
            return entry

    for entry in manifest.get("demos", []):
        if lower_selector in entry.get("name", "").lower():
            return entry

    return None


def command_for_build(entry: dict, preset: str | None) -> list[str]:
    build = entry.get("build", {})
    if not build:
        raise SystemExit("manifest entry missing build metadata")

    if platform_name() == "windows" and "windows" in build:
        build = build["windows"]

    command = build.get("command")
    args = list(build.get("args", []))
    if not command:
        raise SystemExit("manifest entry missing build command")

    build_args = [command] + args

    if preset:
        out = []
        replaced_preset = False
        skip_next = False
        for i, arg in enumerate(build_args):
            if skip_next:
                skip_next = False
                continue
            if arg == "--preset":
                out.append("--preset")
                if i + 1 < len(build_args) and not build_args[i + 1].startswith("--"):
                    out.append(preset)
                    skip_next = True
                else:
                    out.append(preset)
                replaced_preset = True
            elif arg.startswith("--preset="):
                out.append(f"--preset={preset}")
                replaced_preset = True
            else:
                out.append(arg)

        if not replaced_preset:
            out.extend(["--preset", preset])
        build_args = out

    return build_args


def command_for_run(entry: dict) -> list[str]:
    platform_key = platform_name()
    if not entry.get("enabled", True):
        raise SystemExit(f"entry is disabled and marked unavailable: {entry.get('name')} ({entry.get('target')})")

    run = entry.get("run", {}).get(platform_key)
    if not run:
        raise SystemExit(f"missing run command for platform {platform_key}")
    return split_command(resolve_command(run))


def list_entries(manifest: dict) -> None:
    for entry in all_entries(manifest):
        enabled = entry.get("enabled", True)
        state = "enabled" if enabled else "disabled"
        print(f"{entry.get('name')} | id={entry.get('id')} | target={entry.get('target')} | {state}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run examples from generated manifest")
    parser.add_argument("selector", nargs="?", help="example id, target name, or partial name")
    parser.add_argument("--list", action="store_true", help="list available demo entries")
    parser.add_argument("--build", action="store_true", help="build before running")
    parser.add_argument("--build-only", action="store_true", help="build target and exit without launching")
    parser.add_argument("--preset", default=None, help="override cmake preset used for build")
    parser.add_argument("--verbose", action="store_true", help="print executed command")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    manifest = load_manifest()
    if args.list or not args.selector:
        list_entries(manifest)
        return 0

    entry = find_entry(manifest, args.selector)
    if not entry:
        raise SystemExit(f"demo not found: {args.selector}")

    do_build = args.build or args.build_only
    if do_build:
        build_args = command_for_build(entry, args.preset)
        if args.verbose:
            print("build:", " ".join(build_args))
        ret = subprocess.call(build_args, cwd=ROOT)
        if ret != 0:
            return ret
        if args.build_only:
            return ret

    run_args = command_for_run(entry)
    if args.verbose:
        print("run:", " ".join(run_args))
    return subprocess.call(run_args, cwd=ROOT)


if __name__ == "__main__":
    raise SystemExit(main())
