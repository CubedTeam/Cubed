"""Creatures view form editor."""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import loader, paths, validation
from ...core.models import Creature
from .. import form
from .base_view import BaseResourceView


class CreaturesView(BaseResourceView):
    title = "Creatures"
    directory = paths.CREATURES_DIR

    def __init__(self, page: ft.Page) -> None:
        self._field_name = form.field("name")
        self._field_model = form.field("model")
        self._field_anim = form.field("animation (可选)")
        self._field_collision = form.field("collision (可选)")
        super().__init__(page)

    def load_all(self) -> list[dict[str, Any]]:
        return [loader.load_json(p) for p in paths.list_json_files(paths.CREATURES_DIR)]

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.CREATURES_DIR) or {
            "name": "", "model": ""
        }

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        c = Creature.from_dict(data)
        self._field_name.value = c.name
        self._field_model.value = c.model
        self._field_anim.value = c.animation or ""
        self._field_collision.value = c.collision or ""
        return ft.Column(
            [
                form.section("基本", self._field_name),
                form.section("Model", self._field_model),
                form.section("Optional references", self._field_anim, self._field_collision),
            ],
            scroll=ft.ScrollMode.AUTO,
            expand=True,
            spacing=form.SPACE,
        )

    def form_to_data(self) -> dict[str, Any] | None:
        c = Creature(
            name=self._field_name.value or "",
            model=self._field_model.value or "",
            animation=self._field_anim.value or None,
            collision=self._field_collision.value or None,
        )
        return c.to_dict()

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.validate_creature(Creature.from_dict(data))

    def save_data(self, data: dict[str, Any]) -> str:
        c = Creature.from_dict(data)
        loader.save_creature(c)
        return c.name

    def delete_data(self, name: str) -> None:
        loader.delete_creature(name)