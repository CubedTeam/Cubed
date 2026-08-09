"""MD3 themed helpers for the json manager UI.

Green seed (matches Cubed's grass block) drives the color scheme. The
theme toggles between light and dark following Flet's Material 3 mode.
"""

from __future__ import annotations

import flet as ft

SEED = ft.Colors.GREEN_700
CARD_RADIUS = 12
SECTION_RADIUS = 16


def build_theme(mode: ft.ThemeMode) -> ft.Theme:
    return ft.Theme(
        color_scheme_seed=SEED,
        use_material3=True,
    )


def light_theme() -> ft.Theme:
    return build_theme(ft.ThemeMode.LIGHT)


def dark_theme() -> ft.Theme:
    return build_theme(ft.ThemeMode.DARK)


def themed_button(text: str, kind: str = "filled", on_click=None) -> ft.Control:
    """Material 3 button helper. ``kind`` is filled/tonal/outlined/text."""
    if kind == "filled":
        return ft.FilledButton(text, on_click=on_click)
    if kind == "tonal":
        return ft.FilledTonalButton(text, on_click=on_click)
    if kind == "outlined":
        return ft.OutlinedButton(text, on_click=on_click)
    return ft.TextButton(text, on_click=on_click)