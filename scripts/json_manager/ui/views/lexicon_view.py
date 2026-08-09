"""Sensitive lexicon editor (date + word list)."""

from __future__ import annotations

import flet as ft

from ...core import i18n, loader
from .. import form
from ..dialogs import confirm, snack
from ..safe import safe_update


class LexiconView(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True, spacing=form.SPACE, scroll=ft.ScrollMode.AUTO)
        self.page_ctx = page
        self.data: dict = {}

        self.date_field = form.field(i18n.t("view.lexicon.last_update_date_label"))
        self.search = ft.TextField(
            prefix_icon=ft.Icons.SEARCH,
            hint_text=i18n.t("view.lexicon.search_hint"),
            dense=True,
            expand=True,
            on_change=self._on_search,
        )
        self.add_input = form.field(i18n.t("view.lexicon.add_one_label"))
        self.add_btn = ft.FilledTonalButton(
            i18n.t("view.lexicon.add_btn"),
            icon=ft.Icons.ADD,
            on_click=self._on_add,
        )
        self.bulk_input = form.field(
            i18n.t("view.lexicon.bulk_paste_label"),
            multiline=True,
        )
        self.bulk_btn = ft.OutlinedButton(
            i18n.t("view.lexicon.bulk_btn"),
            icon=ft.Icons.UPLOAD,
            on_click=self._on_bulk,
        )
        self.save_btn = ft.FilledButton(
            i18n.t("view.lexicon.save_btn"),
            icon=ft.Icons.SAVE,
            on_click=self._on_save,
        )

        self._chips = ft.Row(
            [],
            wrap=True,
            spacing=6,
            run_spacing=6,
            alignment=ft.MainAxisAlignment.START,
        )
        self.chip_container = ft.Container(
            self._chips,
            padding=form.SECTION_PAD,
            border_radius=form.SECTION_RADIUS,
            bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
        )

        self.controls = [
            form.section(
                i18n.t("view.lexicon.section_metadata"),
                ft.Row([self.date_field, self.save_btn], spacing=form.SPACE),
            ),
            form.section(
                i18n.t("view.lexicon.section_quick_add"),
                ft.Row([self.add_input, self.add_btn], spacing=form.SPACE),
            ),
            form.section(
                i18n.t("view.lexicon.section_bulk_import"),
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
                    label=ft.Text(
                        w, size=12, max_lines=1, overflow=ft.TextOverflow.ELLIPSIS
                    ),
                    on_delete=lambda _e, word=w: self._remove(word),
                    delete_icon=ft.Icon(ft.Icons.CLOSE),
                )
            )
            shown += 1
            if shown > 500:
                chips.append(
                    ft.Text(
                        i18n.t("view.lexicon.more_text", n=len(words) - 500),
                        italic=True,
                        size=12,
                    )
                )
                break
        if not chips:
            chips = [
                ft.Text(
                    i18n.t("view.lexicon.empty_text"),
                    italic=True,
                    color=ft.Colors.ON_SURFACE_VARIANT,
                    size=12,
                )
            ]
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
            snack(self.page_ctx, i18n.t("view.lexicon.already_exists"), "error")
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
        snack(
            self.page_ctx,
            i18n.t("view.lexicon.bulk_imported", n=added),
            "ok",
        )
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
            snack(self.page_ctx, i18n.t("view.lexicon.saved_ok"), "ok")
            self.refresh()

        confirm(
            self.page_ctx,
            i18n.t("view.lexicon.save_title"),
            i18n.t("view.lexicon.save_body"),
            _do,
        )
