"""Raw JSON editor tab — a multiline TextField with monospaced font.

Form and raw views stay in sync via load_from on tab switch to avoid
bi-directional binding complexity.
"""

from __future__ import annotations

import flet as ft


class RawEditor(ft.Column):
    def __init__(self) -> None:
        super().__init__()
        self.field = ft.TextField(
            multiline=True,
            min_lines=20,
            max_lines=40,
            expand=True,
            text_style=ft.TextStyle(font_family="monospace"),
            on_change=self._on_change,
        )
        self.controls = [
            ft.Text("原始 JSON (Raw 编辑绕过表单校验)", weight=ft.FontWeight.BOLD),
            self.field,
        ]
        self.expand = True
        self._raw = "{}"

    def set_value(self, raw: str) -> None:
        self._raw = raw
        self.field.value = raw
        self.field.update()

    def get_value(self) -> str:
        return self.field.value or ""

    def _on_change(self, _e: ft.ControlEvent) -> None:
        self._raw = self.field.value or ""