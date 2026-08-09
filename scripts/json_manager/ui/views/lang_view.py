"""Lang view: three-column editor for localization keys."""

from __future__ import annotations

import flet as ft
from ..safe import safe_update

from ...core import loader
from ..dialogs import confirm, snack


class LangView(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True)
        self.page_ctx = page
        self.data: dict[str, dict[str, str]] = {}
        self.all_keys: list[str] = []
        self.search = ft.TextField(prefix_icon=ft.Icons.SEARCH, hint_text="搜索 key", on_change=self._on_search)
        self._add_btn = ft.FilledTonalButton("新增 key", on_click=self._on_add)
        self._sync_btn = ft.OutlinedButton("从 zh_CN 同步缺失 key 到 en_US", on_click=self._on_sync)
        self._save_btn = ft.FilledButton("保存全部", on_click=self._on_save)
        self._rows: list[ft.Row] = []
        self.table = ft.Column(self._rows, spacing=2)

        self.controls = [
            ft.Row([self.search, self._add_btn, self._sync_btn, self._save_btn], wrap=True),
            ft.Container(
                self.table,
                padding=8,
                border_radius=12,
                bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
                expand=True,
            ),
        ]
        self.refresh()

    def refresh(self) -> None:
        self.data = loader.load_lang_files()
        self.all_keys = sorted(set().union(*(d.keys() for d in self.data.values())))
        self._render("")

    def _render(self, filter_text: str) -> None:
        f = filter_text.lower()
        rows: list[ft.Row] = []
        for key in self.all_keys:
            if f and f not in key.lower():
                continue
            row = ft.Row(
                [
                    ft.Container(ft.Text(key, weight=ft.FontWeight.BOLD), width=260),
                    self._locale_field(key, "zh_CN"),
                    self._locale_field(key, "en_US"),
                    ft.IconButton(
                        ft.Icons.DELETE_OUTLINE,
                        on_click=lambda e, k=key: self._on_delete(k),
                        icon_color=ft.Colors.RED_400,
                    ),
                ],
                alignment=ft.MainAxisAlignment.START,
            )
            rows.append(row)
        self._rows = rows
        self.table.controls = rows
        safe_update(self.table)

    def _locale_field(self, key: str, locale: str) -> ft.TextField:
        val = self.data.get(locale, {}).get(key, "")
        missing = not val
        return ft.TextField(
            value=val,
            dense=True,
            expand=True,
            border_color=ft.Colors.RED_400 if missing else None,
            hint_text=locale + (" (缺失)" if missing else ""),
        )

    def _collect(self) -> None:
        """Read TextField values back into self.data based on current rows."""
        for row in self._rows:
            key = row.controls[0].content.value
            zh_field = row.controls[1]
            en_field = row.controls[2]
            self.data.setdefault("zh_CN", {})[key] = zh_field.value or ""
            self.data.setdefault("en_US", {})[key] = en_field.value or ""

    def _on_search(self, e: ft.ControlEvent) -> None:
        self._render(e.control.value or "")

    def _on_add(self, _e: ft.ControlEvent) -> None:
        from ..dialogs import prompt
        def _add(key: str) -> None:
            key = key.strip()
            if not key:
                return
            if key in self.all_keys:
                snack(self.page_ctx, "key already exists", "error")
                return
            self.all_keys.append(key)
            self.all_keys.sort()
            self._render("")
            snack(self.page_ctx, f"新增 key: {key}", "ok")

        prompt(self.page_ctx, "新增 key", _add, label="key name")

    def _on_delete(self, key: str) -> None:
        def _do():
            self.all_keys.remove(key)
            for locale in self.data:
                self.data[locale].pop(key, None)
            self._render("")
            snack(self.page_ctx, "已删除 " + key, "ok")
        confirm(self.page_ctx, "删除 key", f"删除 {key}? 将从所有 locale 文件移除", _do)

    def _on_sync(self, _e: ft.ControlEvent) -> None:
        zh = self.data.get("zh_CN", {})
        en = self.data.setdefault("en_US", {})
        added = 0
        for k, v in zh.items():
            if k not in en:
                en[k] = v
                added += 1
        snack(self.page_ctx, f"同步完成，新增 {added} 个 key 到 en_US", "ok")
        self._render(self.search.value or "")

    def _on_save(self, _e: ft.ControlEvent) -> None:
        def _do():
            self._collect()
            for locale, d in self.data.items():
                loader.save_lang_file(locale, d)
            snack(self.page_ctx, "已保存全部 lang 文件", "ok")
            self.refresh()
        confirm(self.page_ctx, "保存", "保存所有 lang/*.json?", _do)