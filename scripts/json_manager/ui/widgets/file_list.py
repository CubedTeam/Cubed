"""Sidebar file list widget with search.

Reusable across blocks/items/creatures views; emits the selected file
name via callback. Kept compact and consistent with the MD3 theme.
"""

from __future__ import annotations

from typing import Callable

import flet as ft

from ...core import i18n
from ..safe import safe_update

ITEM_HEIGHT = 40


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
        self.spacing = 8

        self.search = ft.TextField(
            hint_text=i18n.t("widget.file_list.search_hint"),
            prefix_icon=ft.Icons.SEARCH,
            dense=True,
            on_change=self._on_search,
        )
        self.list_view = ft.ListView(
            controls=self._build_items(),
            expand=True,
            spacing=4,
        )
        self.controls = [
            ft.Row(
                [
                    ft.Text(
                        self.title or i18n.t("widget.file_list.default_title"),
                        weight=ft.FontWeight.BOLD,
                        size=16,
                    ),
                    ft.IconButton(
                        ft.Icons.ADD_OUTLINED,
                        on_click=lambda _: on_new() if on_new else None,
                        tooltip=i18n.t("action.new_tip"),
                    ),
                ],
                alignment=ft.MainAxisAlignment.SPACE_BETWEEN,
            ),
            self.search,
            self.list_view,
        ]

    def _build_items(self, filter_text: str = "") -> list[ft.Control]:
        items: list[ft.Control] = []
        f = filter_text.lower()
        for name in self.names:
            if f and f not in name.lower():
                continue
            items.append(
                ft.Container(
                    ft.Row(
                        [
                            ft.Icon(ft.Icons.INSERT_DRIVE_FILE_OUTLINED, size=18),
                            ft.Text(name, expand=True, size=13),
                        ],
                        spacing=8,
                        vertical_alignment=ft.CrossAxisAlignment.CENTER,
                    ),
                    padding=ft.Padding(left=8, right=8, top=8, bottom=8),
                    border_radius=8,
                    on_click=lambda e, n=name: self.on_select(n),
                    ink=True,
                )
            )
        if not items:
            items.append(
                ft.Container(
                    ft.Text(
                        i18n.t("widget.file_list.empty"),
                        italic=True,
                        color=ft.Colors.ON_SURFACE_VARIANT,
                        size=12,
                    ),
                    padding=10,
                    alignment=ft.Alignment.CENTER,
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
