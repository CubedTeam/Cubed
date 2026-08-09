"""Registry view: a clean id-name list for blocks and items.

Read-only by default; unlocking enables inline editing and reorder.
"""

from __future__ import annotations

import flet as ft

from ...core import i18n, loader
from ...core.models import Registry
from .. import form
from ..dialogs import confirm, snack
from ..safe import safe_update


class RegistryView(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True, spacing=form.SPACE, scroll=ft.ScrollMode.AUTO)
        self.page_ctx = page
        self.registry = Registry()
        self.unlocked = False
        self._field_date = form.field(
            i18n.t("view.registry.last_update_date_label"),
            disabled=True,
        )
        self._switch_unlock = ft.Switch(
            label=i18n.t("action.unlock"),
            value=False,
            on_change=self._on_unlock,
        )
        self._save_btn = ft.FilledButton(
            i18n.t("view.registry.save"),
            icon=ft.Icons.SAVE,
            on_click=self._on_save,
            disabled=True,
        )
        self._refresh_btn = ft.OutlinedButton(
            i18n.t("view.registry.refresh"),
            icon=ft.Icons.REFRESH,
            on_click=lambda _: self.refresh(),
        )
        self._blocks_col: ft.Column = ft.Column(spacing=4)
        self._items_col: ft.Column = ft.Column(spacing=4)
        self._block_rows: list[ft.Row] = []
        self._item_rows: list[ft.Row] = []
        self.controls = [
            form.section(
                i18n.t("view.registry.section_controls"),
                ft.Row(
                    [self._switch_unlock, self._save_btn, self._refresh_btn],
                    spacing=form.BUTTON_GAP,
                ),
            ),
            form.section(i18n.t("view.registry.section_blocks"), self._blocks_col),
            form.section(i18n.t("view.registry.section_items"), self._items_col),
        ]
        self.refresh()

    def refresh(self) -> None:
        self.registry = loader.load_registry()
        self._build_grid()

    def _build_grid(self) -> None:
        self._block_rows = self._render_group(self.registry.blocks)
        self._item_rows = self._render_group(self.registry.items)
        self._blocks_col.controls = self._block_rows
        self._items_col.controls = self._item_rows
        safe_update(self._blocks_col)
        safe_update(self._items_col)

    def _render_group(self, mapping: dict[str, int]) -> list[ft.Row]:
        rows: list[ft.Row] = []
        for i, (name, _val) in enumerate(sorted(mapping.items(), key=lambda kv: kv[1])):
            id_text = ft.Container(
                ft.Text(
                    str(i),
                    text_align=ft.TextAlign.CENTER,
                    weight=ft.FontWeight.BOLD,
                    size=13,
                ),
                width=40,
                alignment=ft.Alignment.CENTER,
            )
            # Wrap the TextField with decorative border.
            name_field = ft.TextField(
                value=name,
                dense=True,
                disabled=not self.unlocked,
                expand=True,
            )
            rows.append(
                ft.Row(
                    [id_text, name_field],
                    spacing=form.SPACE,
                    vertical_alignment=ft.CrossAxisAlignment.CENTER,
                )
            )
        return rows

    def _on_unlock(self, e: ft.ControlEvent) -> None:
        self.unlocked = bool(e.control.value)
        self._save_btn.disabled = not self.unlocked
        safe_update(self._save_btn)
        self._build_grid()

    def _on_save(self, _e: ft.ControlEvent) -> None:
        def _do_save():
            new_blocks: dict[str, int] = {}
            for i, row in enumerate(self._block_rows):
                name = row.controls[1].value
                if name:
                    new_blocks[name] = i
            new_items: dict[str, int] = {}
            for i, row in enumerate(self._item_rows):
                name = row.controls[1].value
                if name:
                    new_items[name] = i
            self.registry.blocks = new_blocks
            self.registry.items = new_items
            loader.save_registry(self.registry)
            snack(self.page_ctx, i18n.t("view.registry.saved_ok"), "ok")
            self.refresh()

        confirm(
            self.page_ctx,
            i18n.t("view.registry.confirm_save_title"),
            i18n.t("view.registry.confirm_save_body"),
            _do_save,
        )
