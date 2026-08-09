"""Sensitive lexicon editor (date + word list)."""

from __future__ import annotations

import flet as ft

from ...core import loader
from .. import form
from ..dialogs import confirm, snack
from ..safe import safe_update


class LexiconView(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True, spacing=form.SPACE)
        self.page_ctx = page
        self.data: dict = {}

        self.date_field = form.field("lastUpdateDate (YYYY/MM/DD)")
        self.search = ft.TextField(
            prefix_icon=ft.Icons.SEARCH, hint_text="搜索词", dense=True,
            expand=True, on_change=self._on_search,
        )
        self.add_input = form.field("逐个添加", expand=True)
        self.add_btn = ft.FilledTonalButton("添加", icon=ft.Icons.ADD, on_click=self._on_add)
        self.bulk_input = form.field("批量粘贴 (每行一个)", multiline=True)
        self.bulk_btn = ft.OutlinedButton("导入多行", icon=ft.Icons.UPLOAD, on_click=self._on_bulk)
        self.save_btn = ft.FilledButton("保存", icon=ft.Icons.SAVE, on_click=self._on_save)

        self._chips = ft.Row([], wrap=True, spacing=6, run_spacing=6, alignment=ft.MainAxisAlignment.START)
        self.chip_container = ft.Container(
            self._chips,
            padding=form.SECTION_PAD,
            border_radius=form.SECTION_RADIUS,
            bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
            expand=True,
        )

        self.controls = [
            form.section(
                "元数据",
                ft.Row([self.date_field, self.save_btn], spacing=form.SPACE),
            ),
            form.section(
                "快速添加",
                ft.Row([self.add_input, self.add_btn], spacing=form.SPACE),
            ),
            form.section(
                "批量导入",
                self.bulk_input,
                self.bulk_btn,
            ),
            self.chip_container,
        ]
        self.refresh()

    def refresh(self) -> None:
        self.data = loader.load_lexicon()
        self.date_field.value = self.data.get("lastUpdateDate", "")
        self.render_chips(self.search.value or "")
        safe_update(self.date_field)

    def render_chips(self, filter_text: str) -> None:
        f = filter_text.lower()
        words = self.data.setdefault("words", [])
        chips: list[ft.Control] = []
        shown = 0
        for w in words:
            if f and f not in w.lower():
                continue
            chips.append(
                ft.Chip(
                    label=ft.Text(w, size=12, max_lines=1, overflow=ft.TextOverflow.ELLIPSIS),
                    on_delete=lambda _e, word=w: self._remove(word),
                    delete_icon=ft.Icon(ft.Icons.CLOSE),
                )
            )
            shown += 1
            if shown > 500:
                chips.append(
                    ft.Text(f"… 还有 {len(words) - 500} 项 (请缩小搜索)", italic=True, size=12)
                )
                break
        if not chips:
            chips = [ft.Text("(空)", italic=True, color=ft.Colors.ON_SURFACE_VARIANT, size=12)]
        self._chips.controls = chips
        safe_update(self._chips)

    def _on_search(self, e: ft.ControlEvent) -> None:
        self.render_chips(e.control.value or "")

    def _current_words(self) -> list[str]:
        return self.data.setdefault("words", [])

    def _on_add(self, _e: ft.ControlEvent) -> None:
        w = (self.add_input.value or "").strip()
        if not w:
            return
        words = self._current_words()
        if w in words:
            snack(self.page_ctx, "已存在", "error")
            return
        words.append(w)
        self.add_input.value = ""
        safe_update(self.add_input)
        self.render_chips(self.search.value or "")

    def _on_bulk(self, _e: ft.ControlEvent) -> None:
        raw = self.bulk_input.value or ""
        words = self._current_words()
        existing = set(words)
        added = 0
        for line in raw.splitlines():
            w = line.strip()
            if w and w not in existing:
                words.append(w)
                existing.add(w)
                added += 1
        self.bulk_input.value = ""
        safe_update(self.bulk_input)
        snack(self.page_ctx, f"批量导入 {added} 词", "ok")
        self.render_chips(self.search.value or "")

    def _remove(self, word: str) -> None:
        words = self._current_words()
        if word in words:
            words.remove(word)
        self.render_chips(self.search.value or "")

    def _on_save(self, _e: ft.ControlEvent) -> None:
        def _do():
            self.data["lastUpdateDate"] = self.date_field.value or ""
            loader.save_lexicon(self.data)
            snack(self.page_ctx, "已保存敏感词表", "ok")
            self.refresh()

        confirm(self.page_ctx, "保存", "保存 SensitiveLexicon.json?", _do)