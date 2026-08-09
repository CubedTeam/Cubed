"""Blocks view (schema-driven)."""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import loader, paths, validation
from ...core.models import Block
from ...core.schema import BLOCK_SCHEMA
from .. import form
from .base_view import BaseResourceView


class BlocksView(BaseResourceView):
    title = "Blocks"
    directory = paths.BLOCKS_DIR
    schema = BLOCK_SCHEMA

    def __init__(self, page: ft.Page) -> None:
        self.schema_form = form.SchemaForm(self.schema)
        super().__init__(page)

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.BLOCKS_DIR) or {
            "name": "",
            "properties": {},
            "texture": {"type": "cuboid", "path": ""},
            "sounds": {"break": "", "place": ""},
        }

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        self.schema_form.set_data(data)
        return self.schema_form

    def form_to_data(self) -> dict[str, Any] | None:
        return self.schema_form.get_data()

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.validate_block(data)

    def save_data(self, data: dict[str, Any]) -> str:
        block = Block.from_dict(data)
        registry = loader.load_registry()
        loader.sync_registry_for_block(block, registry)
        loader.save_block(block)
        loader.save_registry(registry)
        return block.name

    def delete_data(self, name: str) -> None:
        loader.delete_block(name)
        registry = loader.load_registry()
        loader.remove_block_from_registry(name, registry)
        loader.save_registry(registry)