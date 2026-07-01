#!/usr/bin/env python3
"""Check that Python reference examples have C++ counterparts.

C++-only examples are allowed because the port includes additional demos for
Qt/C++ specific APIs and composed controls. Missing Python reference examples
still fail the release check.
"""

from __future__ import annotations

from pathlib import Path
from typing import List


ROOT = Path(__file__).resolve().parents[1]
CPP_EXAMPLES = ROOT / "examples"
PY_EXAMPLES = ROOT / "PyQt-Fluent-Widgets" / "examples"


def gather_example_dirs(base: Path, skip_dirs: List[str]) -> set[str]:
    dirs = set()
    for path in base.glob("*/*"):
        if not path.is_dir():
            continue
        if any(skip in path.parts for skip in skip_dirs):
            continue
        if not (path / "CMakeLists.txt").exists():
            continue
        dirs.add(path.relative_to(base).as_posix())
    return dirs


def normalize_py_dirs(base: Path) -> set[str]:
    return {p.relative_to(base).as_posix() for p in base.glob("*/*") if p.is_dir()}


def main() -> int:
    cpp_dirs = gather_example_dirs(CPP_EXAMPLES, ["gallery"])
    py_dirs = {path for path in normalize_py_dirs(PY_EXAMPLES) if not path.startswith("gallery/")}

    missing_in_cpp = sorted(py_dirs - cpp_dirs)
    missing_in_py = sorted(cpp_dirs - py_dirs)

    print(f"C++ example dirs: {len(cpp_dirs)}")
    print(f"Python example dirs: {len(py_dirs)}")

    if missing_in_cpp:
        print("Missing in C++ examples:")
        for item in missing_in_cpp:
            print(f" - {item}")
    if missing_in_py:
        print("Additional C++ examples:")
        for item in missing_in_py:
            print(f" - {item}")

    return 1 if missing_in_cpp else 0


if __name__ == "__main__":
    raise SystemExit(main())
