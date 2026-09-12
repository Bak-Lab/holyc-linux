#!/usr/bin/env python3
"""Translate a deliberately small, C-compatible HolyC subset and compile it."""

from __future__ import annotations

import argparse
import re
import shlex
import subprocess
import sys
from pathlib import Path

ENTRY_RE = re.compile(r"(?m)^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]*;[ \t]*(?://[^\n]*)?$")
UNSUPPORTED_RE = re.compile(
    r"\b(?:InU(?:8|16|32)|OutU(?:8|16|32)|CLI|STI|Reboot|SysHlt)\b"
)


class TranslationError(ValueError):
    pass


def translate(source: str, source_name: str = "<input>") -> str:
    """Return C for a HolyC source file with a final bare entry-point call."""
    if match := UNSUPPORTED_RE.search(source):
        raise TranslationError(
            f"{source_name}: unsupported hardware operation: {match.group(0)}"
        )

    entries = list(ENTRY_RE.finditer(source))
    if not entries:
        raise TranslationError(
            f"{source_name}: expected a final bare entry point such as 'Main;'"
        )

    entry_match = entries[-1]
    entry = entry_match.group(1)
    if source[entry_match.end() :].strip():
        raise TranslationError(f"{source_name}: entry point must be the final statement")

    body = source[: entry_match.start()].rstrip()
    return (
        '#include "holyc.h"\n'
        f'#line 1 "{source_name}"\n'
        f"{body}\n\n"
        "#line 1 \"<holyc-entry>\"\n"
        "int main(int argc, char **argv)\n"
        "{\n"
        "  if (!HCInit(argc, argv))\n"
        "    return 1;\n"
        f"  {entry}();\n"
        "  HCShutdown();\n"
        "  return 0;\n"
        "}\n"
    )


def pkg_config() -> list[str]:
    command = ["pkg-config", "--cflags", "--libs", "sdl2"]
    try:
        result = subprocess.run(command, check=True, text=True, capture_output=True)
    except FileNotFoundError as error:
        raise TranslationError("pkg-config is required but was not found") from error
    except subprocess.CalledProcessError as error:
        raise TranslationError(
            "SDL2 development files were not found (pkg-config package: sdl2)"
        ) from error
    return shlex.split(result.stdout)


def compile_source(
    root: Path, source_path: Path, output: Path, emit_c: Path | None
) -> None:
    source = source_path.read_text(encoding="utf-8")
    generated = translate(source, str(source_path))

    build_dir = root / ".holyc-build"
    build_dir.mkdir(exist_ok=True)
    generated_path = build_dir / f"{source_path.stem}.c"
    generated_path.write_text(generated, encoding="utf-8")

    if emit_c:
        emit_c.parent.mkdir(parents=True, exist_ok=True)
        emit_c.write_text(generated, encoding="utf-8")

    command = [
        "clang",
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wno-strict-prototypes",
        "-I",
        str(root / "runtime" / "include"),
        str(generated_path),
        str(root / "runtime" / "src" / "holyc_runtime.c"),
        "-o",
        str(output),
        *pkg_config(),
    ]
    try:
        subprocess.run(command, check=True)
    except FileNotFoundError as error:
        raise TranslationError("clang is required but was not found") from error
    except subprocess.CalledProcessError as error:
        raise TranslationError(f"clang failed with exit code {error.returncode}") from error


def main(root: Path | None = None) -> int:
    root = root or Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(
        prog="holyc", description="Compile a supported HolyC source file for Linux"
    )
    parser.add_argument("source", type=Path, help="HolyC .HC source file")
    parser.add_argument("-o", "--output", type=Path, help="output executable")
    parser.add_argument("--emit-c", type=Path, help="also save translated C here")
    args = parser.parse_args()

    if not args.source.is_file():
        parser.error(f"source file not found: {args.source}")

    output = args.output or root / ".holyc-build" / args.source.stem
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    try:
        compile_source(root, args.source.resolve(), output, args.emit_c)
    except (OSError, TranslationError) as error:
        print(f"holyc: {error}", file=sys.stderr)
        return 1

    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
