#!/usr/bin/env python3
"""Generate VS Code tasks/launch entries and an examples manifest for gallery demos."""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Optional


ROOT = Path(__file__).resolve().parents[1]
EXAMPLES_DIR = ROOT / "examples"
VSCODE_DIR = ROOT / ".vscode"
MANIFEST_PATH = EXAMPLES_DIR / "examples.json"
BUILD_CACHE_CANDIDATES = [
    ROOT / "build" / "debug" / "CMakeCache.txt",
    ROOT / "build" / "release" / "CMakeCache.txt",
    ROOT / "build" / "CMakeCache.txt",
]
PROJECT_VERSION_RE = re.compile(r"VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)")
CMAKE_PROJECT_RE = re.compile(r"project\([^\)]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)")
OPTIONAL_FEATURES = {
    "fqw_media_media_player_demo": "FQW_HAS_MULTIMEDIA",
    "fqw_window_web_engine_demo": "FQW_HAS_WEBENGINE_WIDGETS",
}


TARGET_RE = re.compile(r"add_executable\s*\(\s*([A-Za-z0-9_]+)", re.MULTILINE)
CMAKE_CACHE_RE = re.compile(r"^([A-Za-z0-9_]+):BOOL=(.*)$", re.MULTILINE)


def title_case(value: str) -> str:
    return " ".join(word.capitalize() for word in value.split("_"))


def demo_label(target: str, rel_dir: Path) -> str:
    if target == "FluentQtWidgetsGallery":
        return "Gallery"

    parts = list(rel_dir.parts)
    return "Demo: " + " ".join(title_case(part) for part in parts)


def discover_targets() -> list[tuple[str, Path, str]]:
    targets: list[tuple[str, Path, str]] = []
    for cmake_file in sorted(EXAMPLES_DIR.rglob("CMakeLists.txt")):
        text = cmake_file.read_text(encoding="utf-8")
        match = TARGET_RE.search(text)
        if not match:
            continue

        target = match.group(1)
        rel_dir = cmake_file.parent.relative_to(ROOT)
        targets.append((target, rel_dir, demo_label(target, cmake_file.parent.relative_to(EXAMPLES_DIR))))

    gallery = [item for item in targets if item[0] == "FluentQtWidgetsGallery"]
    demos = sorted((item for item in targets if item[0] != "FluentQtWidgetsGallery"), key=lambda item: str(item[1]))
    return gallery + demos


def cache_feature_enabled(feature: str) -> Optional[bool]:
    cache_file = next((path for path in BUILD_CACHE_CANDIDATES if path.exists()), None)
    if cache_file is None:
        return None

    content = cache_file.read_text(encoding="utf-8", errors="ignore")
    match = None
    for candidate in CMAKE_CACHE_RE.finditer(content):
        if candidate.group(1) == feature:
            match = candidate
            break
    if not match:
        return None

    value = match.group(2).strip().lower()
    if value in {"1", "on", "true", "yes"}:
        return True
    if value in {"0", "off", "false", "no"}:
        return False
    return None


def detect_project_version() -> str:
    cmake_file = ROOT / "CMakeLists.txt"
    if not cmake_file.exists():
        return "0.1.0"

    content = cmake_file.read_text(encoding="utf-8", errors="ignore")
    match = PROJECT_VERSION_RE.search(content)
    if match:
        return match.group(1)

    match = CMAKE_PROJECT_RE.search(content)
    return match.group(1) if match else "0.1.0"


def build_manifest_entries(target: str, rel_dir: Path, label: str) -> dict:
    required_feature = OPTIONAL_FEATURES.get(target)
    enabled = None
    if required_feature:
        enabled = cache_feature_enabled(required_feature)

    if enabled is None:
        enabled = required_feature is None

    return {
        "name": label,
        "id": rel_dir.as_posix(),
        "target": target,
        "relative_path": rel_dir.as_posix(),
        "optional": required_feature is not None,
        "required_feature": required_feature,
        "enabled": enabled,
        "build": {
            "command": "cmake",
            "args": ["--build", "--preset", "debug", "--target", target, "--parallel"],
            "windows": {
                "command": "cmake",
                "args": ["--build", "--preset", "mingw-debug", "--target", target, "--parallel"],
            },
        },
        "run": {
            "mac": executable_command(target, rel_dir, "mac"),
            "windows": executable_command(target, rel_dir, "windows"),
            "linux": executable_command(target, rel_dir, "linux"),
        },
    }


def manifest_payload(targets: list[tuple[str, Path, str]]) -> dict:
    gallery_item = None
    demos = []

    for target, rel_dir, label in targets:
        item = build_manifest_entries(target, rel_dir, label)
        if target == "FluentQtWidgetsGallery":
            gallery_item = item
            continue
        demos.append(item)

    return {
        "schema_version": 1,
        "gallery": gallery_item,
        "demos": demos,
    }


def build_task(label: str, target: str) -> dict:
    task = {
        "label": f"{label}: Build",
        "type": "shell",
        "command": "cmake",
        "args": ["--build", "--preset", "debug", "--target", target, "--parallel"],
        "options": {"cwd": "${workspaceFolder}"},
        "dependsOn": "CMake: Configure Debug",
        "problemMatcher": "$gcc",
        "windows": {
            "args": ["--build", "--preset", "mingw-debug", "--target", target, "--parallel"],
        },
    }
    return task


def gallery_build_and_run_task() -> dict:
    return {
        "label": "Gallery: Build and Run",
        "type": "shell",
        "command": "open",
        "args": ["${workspaceFolder}/build/debug/examples/gallery/FluentQtWidgetsGallery.app"],
        "options": {"cwd": "${workspaceFolder}"},
        "dependsOn": "Gallery: Build",
        "problemMatcher": [],
        "windows": {
            "command": "${workspaceFolder}\\build\\examples\\gallery\\FluentQtWidgetsGallery.exe",
            "args": [],
        },
        "linux": {
            "command": "${workspaceFolder}/build/debug/examples/gallery/FluentQtWidgetsGallery",
            "args": [],
        },
    }


def executable_command(target: str, rel_dir: Path, platform: str) -> str:
    slash_dir = rel_dir.as_posix()
    win_dir = str(rel_dir).replace("/", "\\")

    if platform == "windows":
        return f'"${{workspaceFolder}}\\build\\{win_dir}\\{target}.exe"'
    if platform == "linux":
        return f'"${{workspaceFolder}}/build/debug/{slash_dir}/{target}"'
    return f'"${{workspaceFolder}}/build/debug/{slash_dir}/{target}.app/Contents/MacOS/{target}"'


def run_config(label: str, target: str, rel_dir: Path) -> dict:
    return {
        "name": f"{label}: Run",
        "type": "node-terminal",
        "request": "launch",
        "command": executable_command(target, rel_dir, "mac"),
        "cwd": "${workspaceFolder}",
        "preLaunchTask": f"{label}: Build",
        "windows": {"command": executable_command(target, rel_dir, "windows")},
        "linux": {"command": executable_command(target, rel_dir, "linux")},
    }


def gallery_open_config() -> dict:
    return {
        "name": "Gallery: Open",
        "type": "node-terminal",
        "request": "launch",
        "command": 'open "${workspaceFolder}/build/debug/examples/gallery/FluentQtWidgetsGallery.app"',
        "cwd": "${workspaceFolder}",
        "preLaunchTask": "Gallery: Build",
        "windows": {"command": '"${workspaceFolder}\\build\\examples\\gallery\\FluentQtWidgetsGallery.exe"'},
        "linux": {"command": '"${workspaceFolder}/build/debug/examples/gallery/FluentQtWidgetsGallery"'},
    }


def gallery_debug_config() -> dict:
    return {
        "name": "Gallery: Debug",
        "type": "cppdbg",
        "request": "launch",
        "program": "${workspaceFolder}/build/debug/examples/gallery/FluentQtWidgetsGallery.app/Contents/MacOS/FluentQtWidgetsGallery",
        "args": [],
        "stopAtEntry": False,
        "cwd": "${workspaceFolder}",
        "environment": [],
        "externalConsole": False,
        "MIMode": "lldb",
        "preLaunchTask": "Gallery: Build",
        "windows": {
            "program": "${workspaceFolder}\\build\\examples\\gallery\\FluentQtWidgetsGallery.exe",
            "MIMode": "gdb",
        },
        "linux": {
            "program": "${workspaceFolder}/build/debug/examples/gallery/FluentQtWidgetsGallery",
            "MIMode": "gdb",
        },
    }


def main() -> None:
    VSCODE_DIR.mkdir(exist_ok=True)
    targets = discover_targets()
    MANIFEST_PATH.write_text(
        json.dumps(manifest_payload(targets), indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )

    vscode_version = detect_project_version()

    tasks = [
        {
            "label": "CMake: Configure Debug",
            "type": "shell",
            "command": "cmake",
            "args": ["--preset", "ninja-debug"],
            "options": {"cwd": "${workspaceFolder}"},
            "windows": {"args": ["--preset", "mingw-debug"]},
            "problemMatcher": [],
        }
    ]
    for target, _, label in targets:
        tasks.append(build_task(label, target))
        if target == "FluentQtWidgetsGallery":
            tasks.append(gallery_build_and_run_task())

    for task in tasks:
        if task["label"] == "Gallery: Build and Run":
            task["group"] = {"kind": "build", "isDefault": True}
            break

    configs = []
    for target, rel_dir, label in targets:
        if target == "FluentQtWidgetsGallery":
            configs.append(run_config(label, target, rel_dir))
            configs.append(gallery_debug_config())
            configs.append(gallery_open_config())
        else:
            configs.append(run_config(label, target, rel_dir))

    (VSCODE_DIR / "tasks.json").write_text(
        json.dumps({"version": vscode_version, "tasks": tasks}, indent=2) + "\n",
        encoding="utf-8",
    )
    (VSCODE_DIR / "launch.json").write_text(
        json.dumps({"version": vscode_version, "configurations": configs}, indent=2) + "\n",
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
