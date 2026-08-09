"""Unified MD3 form primitives for the json manager.

Every view composes its UI from these helpers so spacing, padding,
widths, and typography stay consistent across screens.
"""

from __future__ import annotations

from typing import Any, Iterable

import flet as ft

from ..core.schema import FieldSpec, Schema, get_path, set_path
from .safe import safe_update

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


# --- schema-driven form ----------------------------------------------------


class SchemaForm(ft.Column):
    """Build, populate, and read a flet form from a :class:`Schema`.

    Each FieldSpec becomes one control wrapped in a visibility container.
    Visibility is re-evaluated whenever a segmented button changes, so
    ``visible_when`` rules (item type switch, texture cross) update live.
    """

    def __init__(self, schema: Schema) -> None:
        super().__init__(spacing=SPACE, expand=True, scroll=ft.ScrollMode.AUTO)
        self.schema = schema
        self._data: dict = {}
        # key -> entry dict {control, container, spec, ...extras}
        self._entries: dict[str, dict] = {}
        self.controls = self._build_sections()

    # --- build ------------------------------------------------------------

    def _build_sections(self) -> list[ft.Control]:
        order: list[str] = []
        for f in self.schema.fields:
            if f.section not in order:
                order.append(f.section)
        cols: list[ft.Control] = []
        for sec in order:
            switch_containers: list[ft.Container] = []
            non_switch: list[ft.Container] = []
            for f in self.schema.fields:
                if f.section != sec:
                    continue
                ctrl, extras = self._make_control(f)
                container = ft.Container(ctrl, visible=True)
                entry: dict = {"spec": f, "control": ctrl, "container": container}
                entry.update(extras)
                self._entries[f.key] = entry
                if f.widget == "switch":
                    switch_containers.append(container)
                else:
                    non_switch.append(container)
            section_children: list[ft.Control] = []
            if switch_containers:
                section_children.append(switch_grid([c.content for c in switch_containers]))
            section_children.extend(non_switch)
            cols.append(section(sec, *section_children))
        return cols

    def _make_control(self, f: FieldSpec) -> tuple[ft.Control, dict]:
        if f.widget == "switch":
            return ft.Switch(label=f.label_text, value=bool(f.default)), {}
        if f.widget == "slider":
            slider = ft.Slider(
                min=f.range[0] if f.range else 0.0,
                max=f.range[1] if f.range else 1.0,
                divisions=f.divisions,
                value=float(f.default),
                label="{value}",
                expand=True,
                on_change=lambda e, k=f.key: self._on_slider_change(k, e),
            )
            vlbl = ft.Text(f"{float(f.default):.2f}", size=14, text_align=ft.TextAlign.RIGHT)
            return slider_with_value(slider, vlbl), {"slider": slider, "value_label": vlbl}
        if f.widget == "segmented":
            sel = [f.default] if f.default else []
            seg = ft.SegmentedButton(
                selected=list(sel),
                segments=[ft.Segment(value=opt, label=ft.Text(opt)) for opt in (f.options or [])],
                allow_multiple_selection=False,
                allow_empty_selection=False,
                expand=True,
                on_change=lambda e, k=f.key: self._on_segmented_change(k, e),
            )
            return seg, {}
        return (
            field(
                f.label_text,
                value="" if f.default is None else f.default,
                multiline=f.multiline,
            ),
            {},
        )

    # --- data in / out ----------------------------------------------------

    def set_data(self, data: dict) -> None:
        self._data = data or {}
        for f in self.schema.fields:
            entry = self._entries[f.key]
            ctrl = entry["control"]
            v = get_path(self._data, f.key, f.default)
            if f.widget == "switch":
                ctrl.value = bool(v)
            elif f.widget == "slider":
                val = float(v or 0)
                entry["slider"].value = val
                entry["value_label"].value = f"{val:.2f}"
            elif f.widget == "segmented":
                ctrl.selected = [v] if v else []
            else:
                ctrl.value = "" if v is None else v
        self._apply_visibility()
        safe_update(self)

    def get_data(self) -> dict:
        raw: dict[str, Any] = {}
        for f in self.schema.fields:
            entry = self._entries[f.key]
            ctrl = entry["control"]
            if f.widget == "switch":
                v: Any = bool(ctrl.value)
            elif f.widget == "slider":
                v = float(entry["slider"].value or 0)
            elif f.widget == "segmented":
                sel = ctrl.selected or []
                v = sel[0] if sel else f.default
            else:
                val = ctrl.value
                if f.py_type is str:
                    v = "" if val is None else str(val)
                else:
                    v = None if val is None or val == "" else val
            raw[f.key] = v
        nested: dict[str, Any] = {}
        for k, v in raw.items():
            set_path(nested, k, v)
        final: dict[str, Any] = {}
        for f in self.schema.fields:
            if f.visible_when is not None and not f.visible_when(nested):
                continue
            set_path(final, f.key, raw[f.key])
        return final

    # --- visibility / interactivity --------------------------------------

    def _apply_visibility(self) -> None:
        nested: dict[str, Any] = {}
        for f in self.schema.fields:
            set_path(nested, f.key, get_path(self._data, f.key, f.default))
        for f in self.schema.fields:
            if f.visible_when is None:
                continue
            entry = self._entries[f.key]
            entry["container"].visible = bool(f.visible_when(nested))

    def _on_segmented_change(self, _key: str, _e: ft.ControlEvent) -> None:
        self._data = self.get_data()
        self._apply_visibility()
        safe_update(self)

    def _on_slider_change(self, key: str, e: ft.ControlEvent) -> None:
        v = float(e.control.value or 0)
        entry = self._entries[key]
        entry["value_label"].value = f"{v:.2f}"
        safe_update(entry["value_label"])