"""Resolve asset paths relative to the project root.

The project root is anchored two directories above this file
(scripts/json_manager/core/paths.py -> ../../.. = project root).
"""

from __future__ import annotations

from pathlib import Path

# Project root: scripts/json_manager/core/paths.py -> ../../..
PROJECT_ROOT: Path = Path(__file__).resolve().parents[3]

ASSETS_DIR: Path = PROJECT_ROOT / "assets"

# Cubed game resources (typed json files).
CUBED_DIR: Path = ASSETS_DIR / "cubed"
BLOCKS_DIR: Path = CUBED_DIR / "blocks"
ITEMS_DIR: Path = CUBED_DIR / "items"
CREATURES_DIR: Path = Path(CUBED_DIR) / "creatures"
REGISTRY_FILE: Path = CUBED_DIR / "registry.json"

# Localization files.
LANG_DIR: Path = ASSETS_DIR / "lang"

# Standalone json resources.
LEXICON_FILE: Path = ASSETS_DIR / "SensitiveLexicon.json"

# Backup directory for .bak snapshots.
BACKUP_DIR: Path = ASSETS_DIR / ".bak"


def ensure_dirs() -> None:
    """Create missing managed directories so the tool does not crash
    on a fresh clone."""
    for d in (BLOCKS_DIR, ITEMS_DIR, CREATURES_DIR, LANG_DIR, BACKUP_DIR):
        d.mkdir(parents=True, exist_ok=True)


def list_json_files(directory: Path) -> list[Path]:
    """Return sorted json files in a directory (non-recursive)."""
    if not directory.is_dir():
        return []
    return sorted(p for p in directory.iterdir() if p.is_file() and p.suffix == ".json")
