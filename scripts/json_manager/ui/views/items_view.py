"""Items view form editor."""

from __future__ import annotations

from typing import Any

import flet as ft
from ..safe import safe_update

from ...core import loader, paths, validation
from ...core.models import Item
from .base_view import BaseResourceView


class ItemsView(BaseResourceView):
    title = "Items"
    directory = paths.ITEMS_DIR

    def __init__(self, page: ft.Page) -> None:
        self._field_name = ft.TextField(label="name")
        self._seg_type = ft.SegmentedButton(
            selected=["block"],
            segments=[ft.Segment(value="block", label=ft.Text("block")), ft.Segment(value="spawn_egg", label=ft.Text("spawn_egg"))],
            allow_multiple_selection=False, allow_empty_selection=False,
            on_change=self._on_type_change,
        )
        self._dd_block = ft.Dropdown(label="block")
        self._dd_creature = ft.Dropdown(label="creature", visible=False)
        self._field_texture = ft.TextField(label="texture")
        self._field_desc = ft.TextField(label="description", multiline=True, min_lines=2)
        super().__init__(page)

    def load_all(self) -> list[dict[str, Any]]:
        return [loader.load_json(p) for p in paths.list_json_files(paths.ITEMS_DIR)]

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.ITEMS_DIR) or {
            "name": "", "type": "block", "block": "", "texture": "", "description": "",
        }

    def _populate_dropdowns(self) -> None:
        registry = loader.load_registry()
        self._dd_block.options = [ft.DropdownOption(key=k, text=k) for k in registry.blocks]
        creatures = [c.name for c in loader.load_creatures()]
        self._dd_creature.options = [ft.DropdownOption(key=k, text=k) for k in creatures]

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        self._populate_dropdowns()
        item = Item.from_dict(data)
        self._field_name.value = item.name
        self._seg_type.selected = [item.type]
        self._dd_block.value = item.block or ""
        self._dd_creature.value = item.creature or ""
        self._field_texture.value = item.texture
        self._field_desc.value = item.description
        self._toggle_ref_fields(item.type)
        return ft.Column(
            [
                self._field_name,
                ft.Text("type", weight=ft.FontWeight.BOLD),
                self._seg_type,
                self._dd_block,
                self._dd_creature,
                self._field_texture,
                self._field_desc,
            ],
            scroll=ft.ScrollMode.AUTO,
            expand=True,
        )

    def _toggle_ref_fields(self, kind: str) -> None:
        if kind == "block":
            self._dd_block.visible = True
            self._dd_creature.visible = False
        elif kind == "spawn_egg":
            self._dd_block.visible = False
            self._dd_creature.visible = True
        else:
            self._dd_block.visible = False
            self._dd_creature.visible = False

    def _on_type_change(self, e: ft.ControlEvent) -> None:
        kind = (self._seg_type.selected or ["block"])[0]
        self._toggle_ref_fields(kind)
        safe_update(self._dd_block)
        safe_update(self._dd_creature)

    def form_to_data(self) -> dict[str, Any] | None:
        item = Item(
            name=self._field_name.value or "",
            type=(self._seg_type.selected or ["block"])[0],
            block=self._dd_block.value if self._dd_block.value else None,
            creature=self._dd_creature.value if self._dd_creature.value else None,
            texture=self._field_texture.value or "",
            description=self._field_desc.value or "",
        )
        return item.to_dict()

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.validate_item(Item.from_dict(data))

    def save_data(self, data: dict[str, Any]) -> str:
        item = Item.from_dict(data)
        registry = loader.load_registry()
        loader.sync_registry_for_item(item, registry)
        loader.save_item(item)
        loader.save_registry(registry)
        return item.name

    def delete_data(self, name: str) -> None:
        loader.delete_item(name)
        registry = loader.load_registry()
        loader.remove_item_from_registry(name, registry)
        loader.save_registry(registry)