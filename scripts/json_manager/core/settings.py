"""Persist user settings (currently just the chosen UI locale).

The settings file lives at the project root as ``.json_manager_settings.json``
and is intentionally gitignored so each developer keeps their own choice.
"""

from __future__ import annotations

import json
from typing import Any

from . import paths

SETTINGS_FILE = paths.PROJECT_ROOT / ".json_manager_settings.json"


def load() -> dict[str, Any]:
    """Return the saved settings dict, or ``{}`` when missing/corrupt."""
    if not SETTINGS_FILE.is_file():
        return {}
    try:
        data = json.loads(SETTINGS_FILE.read_text(encoding="utf-8"))
    except OSError, json.JSONDecodeError:
        return {}
    return data if isinstance(data, dict) else {}


def save(data: dict[str, Any]) -> None:
    """Write the settings dict atomically. Best-effort: failures are silent."""
    try:
        SETTINGS_FILE.parent.mkdir(parents=True, exist_ok=True)
        tmp = SETTINGS_FILE.with_suffix(".json.tmp")
        tmp.write_text(
            json.dumps(data, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
        )
        tmp.replace(SETTINGS_FILE)
    except OSError:
        pass
