"""Lang view: a clean three-column key/localization editor."""

from __future__ import annotations

import flet as ft

from ...core import loader
from .. import form
from ..dialogs import confirm, prompt, snack
from ..safe import safe_update


KEY_WIDTH = 280


class LangView(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True, spacing=form.SPACE)
        self.page_ctx = page
        self.data: dict[str, dict[str, str]] = {}
        self.all_keys: list[str] = []

        self.search = ft.TextField(
            prefix_icon=ft.Icons.SEARCH, hint_text="Search key", dense=True, expand=True,
            on_change=self._on_search,
        )
        self._add_btn = ft.FilledTonalButton("New key", icon=ft.Icons.ADD, on_click=self._on_add)
        self._sync_btn = ft.OutlinedButton("Sync zh_CN to en_US", icon=ft.Icons.SYNC, on_click=self._on_sync)
        self._save_btn = ft.FilledButton("Save all", icon=ft.Icons.SAVE, on_click=self._on_save)

        self._header_row = ft.Row(
            [
                ft.Container(ft.Text("Key", weight=ft.FontWeight.BOLD), width=KEY_WIDTH),
                ft.Text("zh_CN", weight=ft.FontWeight.BOLD, expand=True),
                ft.Text("en_US", weight=ft.FontWeight.BOLD, expand=True),
                ft.Container(width=40),  # delete column spacer
            ],
            spacing=form.SPACE,
        )
        self.table = ft.Column(spacing=4, scroll=ft.ScrollMode.AUTO, expand=True)

        self.controls = [
            form.section(
                "Actions",
                ft.Row([self.search, self._add_btn, self._sync_btn, self._save_btn], spacing=form.BUTTON_GAP),
            ),
            ft.Container(self._header_row, padding=ft.Padding(left=0, right=4, top=0, bottom=0)),
            ft.Container(self.table, expand=True, padding=0),
        ]
        self.refresh()

    def refresh(self) -> None:
        self.data = loader.load_lang_files()
        self.all_keys = sorted(set().union(*(d.keys() for d in self.data.values())))
        self._render("")

    def _render(self, filter_text: str) -> None:
        f = filter_text.lower()
        rows: list[ft.Control] = [self._header_row_clone()]
        for key in self.all_keys:
            if f and f not in key.lower():
                continue
            rows.append(self._row_for(key))
        self.table.controls = rows
        safe_update(self.table)

    def _header_row_clone(self) -> ft.Row:
        # Build a new header row instance to avoid ref shared with original.
        return ft.Row(
            [
                ft.Container(ft.Text("Key", weight=ft.FontWeight.BOLD, size=13), width=KEY_WIDTH),
                ft.Text("zh_CN", weight=ft.FontWeight.BOLD, size=13, expand=True),
                ft.Text("en_US", weight=ft.FontWeight.BOLD, size=13, expand=True),
                ft.Container(width=40),
            ],
            spacing=form.SPACE,
            vertical_alignment=ft.CrossAxisAlignment.CENTER,
        )

    def _row_for(self, key: str) -> ft.Row:
        zh_val = self.data.get("zh_CN", {}).get(key, "")
        en_val = self.data.get("en_US", {}).get(key, "")
        zh_missing = not zh_val
        en_missing = not en_val
        return ft.Row(
            [
                ft.Container(
                    ft.Text(key, size=12, weight=ft.FontWeight.BOLD),
                    width=KEY_WIDTH,
                    alignment=ft.Alignment.CENTER_LEFT,
                ),
                ft.TextField(
                    value=zh_val, dense=True, expand=True,
                    border_color=ft.Colors.RED_400 if zh_missing else None,
                    hint_text="(missing)" if zh_missing else None,
                ),
                ft.TextField(
                    value=en_val, dense=True, expand=True,
                    border_color=ft.Colors.RED_400 if en_missing else None,
                    hint_text="(missing)" if en_missing else None,
                ),
                ft.IconButton(
                    ft.Icons.DELETE_OUTLINE,
                    icon_color=ft.Colors.RED_400,
                    on_click=lambda _e, k=key: self._on_delete(k),
                ),
            ],
            spacing=form.SPACE,
            vertical_alignment=ft.CrossAxisAlignment.CENTER,
        )

    def _collect(self) -> None:
        # Re-read values from rendered rows (skip header at index 0).
        for row in self.table.controls[1:]:
            row_key = row.controls[0].content.value
            zh_field = row.controls[1]
            en_field = row.controls[2]
            self.data.setdefault("zh_CN", {})[row_key] = zh_field.value or ""
            self.data.setdefault("en_US", {})[row_key] = en_field.value or ""

    def _on_search(self, e: ft.ControlEvent) -> None:
        self._render(e.control.value or "")

    def _on_add(self, _e: ft.ControlEvent) -> None:
        def _add(key: str) -> None:
            key = key.strip()
            if not key:
                return
            if key in self.all_keys:
                snack(self.page_ctx, "key already exists", "error")
                return
            self.all_keys.append(key)
            self.all_keys.sort()
            self._render(self.search.value or "")
            snack(self.page_ctx, f"Added key: {key}", "ok")

        prompt(self.page_ctx, "New key", _add, label="key name")

    def _on_delete(self, key: str) -> None:
        def _do():
            self.all_keys.remove(key)
            for locale in self.data:
                self.data[locale].pop(key, None)
            self._render(self.search.value or "")
            snack(self.page_ctx, "Deleted " + key, "ok")
        confirm(self.page_ctx, "Delete key", f"Delete {key}? It will be removed from all locale files", _do)

    def _on_sync(self, _e: ft.ControlEvent) -> None:
        zh = self.data.get("zh_CN", {})
        en = self.data.setdefault("en_US", {})
        added = 0
        for k, v in zh.items():
            if k not in en:
                en[k] = v
                added += 1
        snack(self.page_ctx, f"Sync done, added {added} keys to en_US", "ok")
        self._render(self.search.value or "")

    def _on_save(self, _e: ft.ControlEvent) -> None:
        def _do():
            self._collect()
            for locale, d in self.data.items():
                loader.save_lang_file(locale, d)
            snack(self.page_ctx, "Saved all lang files", "ok")
            self.refresh()
        confirm(self.page_ctx, "Save", "Save all lang/*.json?", _do)