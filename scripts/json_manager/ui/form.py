"""Unified MD3 form primitives for the json manager.

Every view composes its UI from these helpers so spacing, padding,
 widths, and typography stay consistent across screens.
"""

from __future__ import annotations

from typing import Iterable

import flet as ft

# Unified spacing tokens (MD3 style).
SPACE = 12
SECTION_PAD = 16
SECTION_RADIUS = 12
BUTTON_GAP = 8
SWITCH_WIDTH = 220
VALUE_WIDTH = 72
LABEL_SIZE = 13


def section(title: str, *controls: ft.Control) -> ft.Container:
    """A titled surface card holding related form controls."""
    return ft.Container(
        ft.Column(
            [ft.Text(title, size=16, weight=ft.FontWeight.BOLD), *controls],
            spacing=SPACE,
        ),
        padding=SECTION_PAD,
        border_radius=SECTION_RADIUS,
        bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
    )


def field(label: str, value="", multiline=False, disabled=False, expand=True, **kw) -> ft.TextField:
    """A consistently styled MD3 TextField (dense + full width)."""
    return ft.TextField(
        label=label,
        value=value,
        dense=True,
        expand=expand,
        multiline=multiline,
        min_lines=3 if multiline else None,
        max_lines=6 if multiline else 1,
        disabled=disabled,
        **kw,
    )


def row(*controls: ft.Control) -> ft.Row:
    """A row of equal-width fields; each child should expand=True."""
    return ft.Row(list(controls), spacing=SPACE)


def switch(label: str, value: bool = False) -> ft.Switch:
    return ft.Switch(label=label, value=value)


def switch_grid(switches: Iterable[ft.Switch], per_row: int = 2) -> ft.Column:
    """Wrap switches into a tidy grid; each switch gets a fixed width."""
    items = list(switches)
    rows: list[ft.Control] = []
    for i in range(0, len(items), per_row):
        chunk = items[i : i + per_row]
        cells = [ft.Container(s, width=SWITCH_WIDTH) for s in chunk]
        rows.append(ft.Row(cells, spacing=SPACE))
    return ft.Column(rows, spacing=SPACE)


def slider_with_value(slider: ft.Slider, label_text: ft.Text) -> ft.Row:
    """Slider fills remaining width; value label keeps a fixed width."""
    return ft.Row(
        [slider, ft.Container(label_text, width=VALUE_WIDTH, alignment=ft.Alignment.CENTER_RIGHT)],
        spacing=SPACE,
    )


def action_bar(*buttons: ft.Control) -> ft.Row:
    """Right-aligned action button row with consistent spacing."""
    return ft.Row(list(buttons), alignment=ft.MainAxisAlignment.END, spacing=BUTTON_GAP)


def labeled(text: str) -> ft.Text:
    """A consistent field label header."""
    return ft.Text(text, size=LABEL_SIZE, color=ft.Colors.ON_SURFACE_VARIANT)