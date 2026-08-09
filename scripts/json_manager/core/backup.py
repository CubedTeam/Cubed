"""Automatic .bak snapshots before any managed json file is overwritten.

Backups land in assets/.bak mirroring the source relative path, with a
timestamp suffix. A rolling limit keeps the disk footprint bounded.
"""

from __future__ import annotations

import shutil
from datetime import datetime
from pathlib import Path

from .paths import BACKUP_DIR, PROJECT_ROOT

MAX_BACKUPS_PER_FILE = 10


def backup_file(src: Path) -> Path | None:
    """Copy ``src`` into the backup tree, return the backup path or None
    when the source does not exist (nothing to back up)."""
    if not src.is_file():
        return None

    rel = src.relative_to(PROJECT_ROOT)
    ts = datetime.now().strftime("%Y%m%d-%H%M%S")
    dest = BACKUP_DIR / f"{rel.as_posix()}.{ts}.bak"
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dest)
    _prune(dest.parent, src.stem, rel.as_posix())
    return dest


def _prune(backup_dir: Path, _stem: str, rel_posix: str) -> None:
    """Keep only the newest ``MAX_BACKUPS_PER_FILE`` snapshots for a
    given source file."""
    prefix = rel_posix + "."
    siblings = sorted(
        (p for p in backup_dir.parent.rglob("*.bak") if p.name.startswith(prefix)),
        key=lambda p: p.stat().st_mtime,
        reverse=True,
    )
    for old in siblings[MAX_BACKUPS_PER_FILE:]:
        try:
            old.unlink()
        except OSError:
            pass


def is_backup_path(path: Path) -> bool:
    return ".bak" in path.parts or path.suffix == ".bak"