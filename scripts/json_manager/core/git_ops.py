"""Git helpers used by the status bar and diff dialog.

All git operations run as subprocesses against the project repo. The
tool never force-pushes or rewrites history; it only surfaces `status`,
renders `diff`, and offers a `commit` shortcut.
"""

from __future__ import annotations

import subprocess
from dataclasses import dataclass
from pathlib import Path

from loguru import logger

from .paths import PROJECT_ROOT


@dataclass
class GitFile:
    path: Path
    status: str  # raw porcelain code, e.g. "M", "??", "A "
    tracked: bool


def _run(args: list[str]) -> str:
    try:
        proc = subprocess.run(
            args,
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        if proc.returncode != 0:
            logger.debug(f"git {' '.join(args)} -> {proc.stderr.strip()}")
        return proc.stdout
    except FileNotFoundError:
        return ""


def is_repo() -> bool:
    out = _run(["git", "rev-parse", "--is-inside-work-tree"])
    return out.strip() == "true"


def status() -> list[GitFile]:
    out = _run(["git", "status", "--porcelain"])
    files: list[GitFile] = []
    for line in out.splitlines():
        if not line.strip():
            continue
        code = line[:2]
        relpath = line[3:].strip()
        # git quotes paths with special chars; strip the quotes.
        if relpath.startswith('"') and relpath.endswith('"'):
            relpath = relpath[1:-1]
        files.append(
            GitFile(
                path=PROJECT_ROOT / relpath,
                status=code.strip(),
                tracked=not code.startswith("??"),
            )
        )
    return files


def diff(path: Path) -> str:
    rel = path.relative_to(PROJECT_ROOT).as_posix()
    return _run(["git", "diff", "--", rel]) or _run(
        ["git", "diff", "--cached", "--", rel]
    )


def add(paths: list[Path]) -> None:
    if not paths:
        return
    rels = [p.relative_to(PROJECT_ROOT).as_posix() for p in paths]
    _run(["git", "add", *rels])


def commit(message: str, paths: list[Path]) -> bool:
    if not message.strip():
        return False
    add(paths)
    proc = subprocess.run(
        ["git", "commit", "-m", message],
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        logger.error(f"git commit failed: {proc.stderr.strip()}")
        return False
    return True


def branch() -> str:
    return _run(["git", "branch", "--show-current"]).strip()
