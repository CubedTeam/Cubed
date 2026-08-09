"""Items view form editor."""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import loader, paths, validation
from ...core.models import Item
from .. import form
from ..safe import safe_update
from .base_view import BaseResourceView


class ItemsView(BaseResourceView):
    title = "Items"
    directory = paths.ITEMS_DIR

    def __init__(self, page: ft.Page) -> None:
        self._field_name = form.field("name")
        self._seg_type = ft.SegmentedButton(
            selected=["block"],
            segments=[
                ft.Segment(value="block", label=ft.Text("block")),
                ft.Segment(value="spawn_egg", label=ft.Text("spawn_egg")),
            ],
            allow_multiple_selection=False, allow_empty_selection=False,
            expand=True, on_change=self._on_type_change,
        )
        self._dd_block = form.field("block reference")
        # Note: Dropdown autofill requires options; treated like a TextField
        # here for simplicity and visual consistency.
        self._block_options: list[str] = []
        self._creature_options: list[str] = []
        self._field_block = form.field("block (cubed:<name>)")
        self._field_creature = form.field("creature (cubed:<name>)")
        self._field_texture = form.field("texture")
        self._field_desc = form.field("description", multiline=True)
        super().__init__(page)

    # --- subclass hooks ---------------------------------------------------

    def load_all(self) -> list[dict[str, Any]]:
        return [loader.load_json(p) for p in paths.list_json_files(paths.ITEMS_DIR)]

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.ITEMS_DIR) or {
            "name": "", "type": "block", "block": "", "texture": "", "description": "",
        }

    def _populate_options(self) -> None:
        registry = loader.load_registry()
        self._block_options = list(registry.blocks.keys())
        self._creature_options = [c.name for c in loader.load_creatures()]

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        self._populate_options()
        item = Item.from_dict(data)
        self._field_name.value = item.name
        self._seg_type.selected = [item.type]
        self._field_block.value = item.block or ""
        self._field_creature.value = item.creature or ""
        self._field_texture.value = item.texture
        self._field_desc.value = item.description

        ref_block = form.section("Block reference", self._field_block)
        ref_creature = form.section("Creature reference", self._field_creature)
        ref_block.visible = (item.type == "block")
        ref_creature.visible = (item.type == "spawn_egg")

        return ft.Column(
            [
                form.section("基本", self._field_name),
                form.section(
                    "Type",
                    self._seg_type,
                    form.labeled("block -> uses block reference; spawn_egg -> uses creature reference"),
                ),
                ref_block,
                ref_creature,
                form.section("Texture & description", self._field_texture, self._field_desc),
            ],
            scroll=ft.ScrollMode.AUTO,
            expand=True,
            spacing=form.SPACE,
        )

    def _on_type_change(self, _e: ft.ControlEvent) -> None:
        # Rebuild form so ref sections show/hide immediately.
        kind = (self._seg_type.selected or ["block"])[0]
        # Persist current text fields into current_data before rebuild.
        self.current_data = self.form_to_data() or self.current_data
        if self.current_data.get("type") != kind:
            self.current_data["type"] = kind
            # Clear the other reference when switching type.
            self.current_data.pop("block" if kind == "spawn_egg" else "creature", None)
            self.current_data["block" if kind == "block" else "creature"] = self.current_data.get(
                "block", ""
            ) if kind == "block" else self.current_data.get("creature", "")
        self._render("form")
        safe_update(self.content_slot)

    def form_to_data(self) -> dict[str, Any] | None:
        kind = (self._seg_type.selected or ["block"])[0]
        item = Item(
            name=self._field_name.value or "",
            type=kind,
            block=self._field_block.value or None if kind == "block" else None,
            creature=self._field_creature.value or None if kind == "spawn_egg" else None,
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