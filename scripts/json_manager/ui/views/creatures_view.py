"""Creatures view (schema-driven)."""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import loader, paths, validation
from ...core.models import Creature
from ...core.schema import CREATURE_SCHEMA
from .. import form
from .base_view import BaseResourceView


class CreaturesView(BaseResourceView):
    title = "Creatures"
    directory = paths.CREATURES_DIR
    schema = CREATURE_SCHEMA

    def __init__(self, page: ft.Page) -> None:
        self.schema_form = form.SchemaForm(self.schema)
        super().__init__(page)

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.CREATURES_DIR) or {
            "name": "",
            "model": "",
        }

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        self.schema_form.set_data(data)
        return self.schema_form

    def form_to_data(self) -> dict[str, Any] | None:
        return self.schema_form.get_data()

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.validate_creature(data)

    def save_data(self, data: dict[str, Any]) -> str:
        c = Creature.from_dict(data)
        loader.save_creature(c)
        return c.name

    def delete_data(self, name: str) -> None:
        loader.delete_creature(name)