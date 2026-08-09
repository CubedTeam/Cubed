"""Sidebar file list widget with search.

Reusable across blocks/items/creatures views; emits the selected file
name via callback.
"""

from __future__ import annotations

from typing import Callable

import flet as ft

from ..safe import safe_update


class FileList(ft.Column):
    def __init__(
        self,
        names: list[str],
        on_select: Callable[[str], None],
        on_new: Callable[[], None] | None = None,
        title: str = "",
    ) -> None:
        super().__init__()
        self.names = list(names)
        self.on_select = on_select
        self.on_new = on_new
        self.title = title
        self.search = ft.TextField(
            hint_text="搜索",
            prefix_icon=ft.Icons.SEARCH,
            dense=True,
            on_change=self._on_search,
        )
        self.list_view = ft.ListView(
            controls=self._build_items(),
            expand=True,
            spacing=2,
        )
        self.controls = [
            ft.Row(
                [
                    ft.Text(self.title, weight=ft.FontWeight.BOLD),
                    ft.IconButton(
                        ft.Icons.ADD, on_click=lambda _: on_new() if on_new else None
                    ),
                ],
                alignment=ft.MainAxisAlignment.SPACE_BETWEEN,
            ),
            self.search,
            self.list_view,
        ]
        self.spacing = 6

    def _build_items(self, filter_text: str = "") -> list[ft.Control]:
        items: list[ft.Control] = []
        f = filter_text.lower()
        for name in self.names:
            if f and f not in name.lower():
                continue
            items.append(
                ft.ListTile(
                    leading=ft.Icon(ft.Icons.INSERT_DRIVE_FILE_OUTLINED),
                    title=ft.Text(name),
                    on_click=lambda e, n=name: self.on_select(n),
                )
            )
        if not items:
            items.append(
                ft.Container(
                    content=ft.Text("(空)", italic=True, color=ft.Colors.ON_SURFACE_VARIANT),
                    padding=10,
                )
            )
        return items

    def _on_search(self, e: ft.ControlEvent) -> None:
        text = e.control.value or ""
        self.list_view.controls = self._build_items(text)
        safe_update(self.list_view)

    def refresh(self, names: list[str]) -> None:
        self.names = list(names)
        self.list_view.controls = self._build_items(self.search.value or "")
        safe_update(self.list_view)

    def select(self, name: str | None) -> None:
        for item in self.list_view.controls:
            if isinstance(item, ft.ListTile) and item.title.value == name:
                item.selected = True
            else:
                continue
        safe_update(self.list_view)