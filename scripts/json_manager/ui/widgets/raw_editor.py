"""Raw JSON editor pane: a monospaced multiline TextField filling its area.

The editor is rebuilt into its host's content area when flipped to raw
mode (instead of using `visible`) to avoid stale-height bugs.
"""

from __future__ import annotations

import flet as ft

RAW_STYLE = ft.TextStyle(font_family="monospace", size=13)
RAW_MIN_LINES = 28


class RawEditor(ft.Column):
    def __init__(self) -> None:
        super().__init__()
        self.expand = True
        self.spacing = 8
        self._hint = ft.Text(
            "Raw JSON (Tab 在表单视图与原始视图间同步)",
            size=12,
            color=ft.Colors.ON_SURFACE_VARIANT,
        )
        self.field = ft.TextField(
            multiline=True,
            min_lines=RAW_MIN_LINES,
            max_lines=RAW_MIN_LINES,
            expand=True,
            text_style=RAW_STYLE,
        )
        self.controls = [self._hint, self.field]

    def set_value(self, raw: str) -> None:
        self._raw = raw
        self.field.value = raw
        try:
            self.field.update()
        except Exception:
            pass

    def get_value(self) -> str:
        return self.field.value or ""