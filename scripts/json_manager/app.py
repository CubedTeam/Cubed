"""Flet app entrypoint for the Cubed json resource manager."""

from __future__ import annotations

import flet as ft

from .core import i18n, paths
from .ui import theme
from .ui.shell import Shell

# AI-generated: wrap Control.update so construction-time calls don't crash
# before the control tree is mounted on the page.
_orig_update = ft.Control.update
_orig_update_async = getattr(ft.Control, "update_async", None)


def _safe_update(self):
    try:
        return _orig_update(self)
    except Exception:
        return None


def _safe_update_async(self, *a, **kw):
    if _orig_update_async is None:
        return None
    try:
        return _orig_update_async(self, *a, **kw)
    except Exception:
        return None


ft.Control.update = _safe_update
if _orig_update_async is not None:
    ft.Control.update_async = _safe_update_async


def main(page: ft.Page) -> None:
    paths.ensure_dirs()
    page.title = i18n.t("app.title")
    page.theme_mode = ft.ThemeMode.SYSTEM
    page.theme = theme.build_theme(ft.ThemeMode.SYSTEM)
    page.padding = 10
    page.horizontal_alignment = ft.CrossAxisAlignment.STRETCH

    shell = Shell(page)
    page.add(shell)