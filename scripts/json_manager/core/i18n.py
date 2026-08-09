"""UI localization for the json manager tool.

Strings live in JSON files under ``scripts/json_manager/locales/`` so
translators can add a locale without touching Python. ``t()`` falls back
to English and then to the caller's default / key so the tool keeps
rendering when a translation is unfinished.
"""

from __future__ import annotations

import json
from pathlib import Path

LOCALES_DIR: Path = Path(__file__).resolve().parents[1] / "locales"
DEFAULT_LOCALE: str = "en_US"

_current: str = DEFAULT_LOCALE
_cache: dict[str, dict[str, str]] = {}


def _load(locale: str) -> dict[str, str]:
    if locale in _cache:
        return _cache[locale]
    path = LOCALES_DIR / f"{locale}.json"
    if path.is_file():
        try:
            with path.open("r", encoding="utf-8") as f:
                _cache[locale] = json.load(f)
        except OSError, json.JSONDecodeError:
            _cache[locale] = {}
    else:
        _cache[locale] = {}
    return _cache[locale]


def available() -> list[str]:
    """List all locale codes that have a JSON file in locales/."""
    if not LOCALES_DIR.is_dir():
        return [DEFAULT_LOCALE]
    return sorted(p.stem for p in LOCALES_DIR.glob("*.json") if p.is_file())


def get_locale() -> str:
    return _current


def set_locale(locale: str) -> bool:
    """Switch the active locale. Returns True if accepted."""
    global _current
    if locale == _current:
        return True
    if locale in available():
        _current = locale
        return True
    return False


def t(key: str, default: str | None = None, **kwargs: object) -> str:
    """Look up a string in the current locale.

    Resolution order: current locale -> English (en_US) -> ``default`` ->
    the key itself. When ``kwargs`` are given the result is run through
    ``str.format(**kwargs)`` so strings may carry ``{name}`` placeholders.
    """
    en = _load(DEFAULT_LOCALE)
    val = _load(_current).get(key) or en.get(key) or default or key
    if kwargs:
        try:
            val = val.format(**kwargs)
        except KeyError, IndexError, ValueError:
            pass
    return val
