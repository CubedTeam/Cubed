"""Items view (schema-driven)."""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import loader, paths, validation
from ...core.models import Item
from ...core.schema import ITEM_SCHEMA
from .. import form
from .base_view import BaseResourceView


class ItemsView(BaseResourceView):
    title = "Items"
    directory = paths.ITEMS_DIR
    schema = ITEM_SCHEMA

    def __init__(self, page: ft.Page) -> None:
        self.schema_form = form.SchemaForm(self.schema)
        super().__init__(page)

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.ITEMS_DIR) or {
            "name": "",
            "type": "block",
            "block": "",
            "texture": "",
            "description": "",
        }

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        self.schema_form.set_data(data)
        return self.schema_form

    def form_to_data(self) -> dict[str, Any] | None:
        return self.schema_form.get_data()

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.validate_item(data)

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