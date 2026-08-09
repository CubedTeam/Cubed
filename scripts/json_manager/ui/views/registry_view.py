"""Registry view: shows blocks/items id mappings.

Read-only by default to discourage casual id shuffling. Unlock button
enables re-ordering and renaming.
"""

from __future__ import annotations


import flet as ft
from ..safe import safe_update

from ...core import loader
from ...core.models import Registry
from ..dialogs import confirm, snack


class RegistryView(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True)
        self.page_ctx = page
        self.registry = Registry()
        self.unlocked = False
        self._unlock = ft.Switch(label="解锁编辑", value=False, on_change=self._on_unlock)
        self._save = ft.FilledButton("保存", on_click=self._on_save, disabled=True)
        self._blocks_table = ft.Column()
        self._items_table = ft.Column()
        self._blocks_rows: list[ft.Row] = []
        self._items_rows: list[ft.Row] = []
        self.controls = [
            ft.Row(
                [
                    self._unlock,
                    self._save,
                    ft.OutlinedButton("刷新", on_click=lambda _: self.refresh()),
                ],
                alignment=ft.MainAxisAlignment.END,
            ),
            ft.Text("Registry (registry.json)", size=24, weight=ft.FontWeight.BOLD),
            ft.Container(ft.Column([ft.Text("Blocks", size=18, weight=ft.FontWeight.BOLD), self._blocks_table]), padding=10, border_radius=12, bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH),
            ft.Container(ft.Column([ft.Text("Items", size=18, weight=ft.FontWeight.BOLD), self._items_table]), padding=10, border_radius=12, bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH),
        ]
        self.refresh()

    def refresh(self) -> None:
        self.registry = loader.load_registry()
        self._blocks_rows = self._render_group(self.registry.blocks)
        self._items_rows = self._render_group(self.registry.items)
        self._blocks_table.controls = self._blocks_rows
        self._items_table.controls = self._items_rows
        safe_update(self._blocks_table)
        safe_update(self._items_table)

    def _render_group(self, mapping: dict[str, int]) -> list[ft.Row]:
        rows: list[ft.Row] = []
        for name, val in sorted(mapping.items(), key=lambda kv: kv[1]):
            field = ft.TextField(value=name, dense=True, disabled=not self.unlocked, width=260)
            id_field = ft.Text(str(val), width=40)
            rows.append(ft.Row([field, id_field]))
        return rows

    def _on_unlock(self, e: ft.ControlEvent) -> None:
        self.unlocked = bool(e.control.value)
        self._save.disabled = not self.unlocked
        self._save.update()
        self.refresh()

    def _on_save(self, _e: ft.ControlEvent) -> None:
        def _do_save():
            new_blocks: dict[str, int] = {}
            for i, row in enumerate(self._blocks_rows):
                name = row.controls[0].value
                if name:
                    new_blocks[name] = i
            new_items: dict[str, int] = {}
            for i, row in enumerate(self._items_rows):
                name = row.controls[0].value
                if name:
                    new_items[name] = i
            self.registry.blocks = new_blocks
            self.registry.items = new_items
            loader.save_registry(self.registry)
            snack(self.page_ctx, "已保存 registry.json", "ok")
            self.refresh()

        confirm(self.page_ctx, "保存 registry", "确认覆盖 registry.json (id 顺序保持当前显示)?", _do_save)