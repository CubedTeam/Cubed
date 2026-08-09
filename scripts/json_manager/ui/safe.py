"""Safe update helpers: skip update() when a control is not yet attached
to a live flet page (e.g. during initial construction)."""

from __future__ import annotations

from typing import Iterable

import flet as ft


def safe_update(control: ft.Control) -> None:
    try:
        control.update()
    except Exception:
        pass


def safe_update_all(controls: Iterable[ft.Control]) -> None:
    for c in controls:
        safe_update(c)