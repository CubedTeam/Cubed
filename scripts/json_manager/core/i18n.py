"""Small i18n stub for the tool UI itself.

Falls back to English when a key is missing so the tool always renders
even if a translator has not finished a locale yet.
"""

from __future__ import annotations

_STRINGS: dict[str, dict[str, str]] = {
    "zh_CN": {
        "app.title": "Cubed Resource Manager",
        "nav.blocks": "Blocks",
        "nav.items": "Items",
        "nav.creatures": "Creatures",
        "nav.registry": "Registry",
        "nav.lang": "Lang",
        "nav.lexicon": "Lexicon",
        "action.new": "New",
        "action.save": "Save",
        "action.delete": "Delete",
        "action.duplicate": "Duplicate",
        "action.refresh": "Refresh",
        "action.commit": "Commit",
        "action.view_diff": "View Diff",
        "action.unlock": "Unlock editing",
        "status.modified": "modified",
        "status.untracked": "untracked",
        "status.clean": "working tree clean",
        "msg.saved": "Saved",
        "msg.deleted": "Deleted",
        "msg.invalid": "Validation failed",
        "msg.confirm_delete": "Confirm delete?",
        "view.form": "Form",
        "view.raw": "Raw JSON",
    },
    "en_US": {
        "app.title": "Cubed Resources",
        "nav.blocks": "Blocks",
        "nav.items": "Items",
        "nav.creatures": "Creatures",
        "nav.registry": "Registry",
        "nav.lang": "Lang",
        "nav.lexicon": "Lexicon",
        "action.new": "New",
        "action.save": "Save",
        "action.delete": "Delete",
        "action.duplicate": "Duplicate",
        "action.refresh": "Refresh",
        "action.commit": "Commit",
        "action.view_diff": "View Diff",
        "action.unlock": "Unlock",
        "status.modified": "modified",
        "status.untracked": "untracked",
        "status.clean": "clean",
        "msg.saved": "Saved",
        "msg.deleted": "Deleted",
        "msg.invalid": "Validation failed",
        "msg.confirm_delete": "Confirm delete?",
        "view.form": "Form",
        "view.raw": "Raw JSON",
    },
}

_current = "zh_CN"


def set_locale(locale: str) -> None:
    global _current
    if locale in _STRINGS:
        _current = locale


def t(key: str, default: str | None = None) -> str:
    return _STRINGS.get(_current, {}).get(key, default or key)