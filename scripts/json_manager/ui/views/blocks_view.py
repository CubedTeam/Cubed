"""Blocks view form editor."""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import loader, paths, validation
from ...core.models import Block, BlockProperties, Sounds, Texture
from .base_view import BaseResourceView


class BlocksView(BaseResourceView):
    title = "Blocks"
    directory = paths.BLOCKS_DIR
    name_field_label = "name"

    def __init__(self, page: ft.Page) -> None:
        self._field_name = ft.TextField(label="name")
        self._field_roughness_value = ft.Text("0.75")
        self._field_roughness = ft.Slider(
            min=0.0, max=1.0, divisions=20, value=0.75, label="{value}",
            on_change=self._on_rough_change,
        )
        self._bool_switches: dict[str, ft.Switch] = {
            k: ft.Switch(label=k)
            for k in (
                "is_liquid",
                "is_cross_plane",
                "is_transparent",
                "is_passable",
                "is_discard",
                "is_blend",
                "is_transitional",
                "is_gas",
            )
        }
        self._seg_type = ft.SegmentedButton(
            selected=["cuboid"],
            segments=[
ft.Segment(value="cuboid", label=ft.Text("cuboid"), icon=ft.Icon(ft.Icons.BLOCK)),
            ft.Segment(value="cross", label=ft.Text("cross"), icon=ft.Icon(ft.Icons.GRID_VIEW)),
            ],
            allow_multiple_selection=False,
            allow_empty_selection=False,
        )
        self._field_tex_path = ft.TextField(label="texture.path")
        self._field_normal = ft.TextField(label="texture.normal (可选)", disabled=False)
        self._field_break = ft.TextField(label="sounds.break")
        self._field_place = ft.TextField(label="sounds.place")
        self._field_walk = ft.TextField(label="sounds.walk (可选)")
        super().__init__(page)

    # --- subclass hooks ---------------------------------------------------

    def load_all(self) -> list[dict[str, Any]]:
        return [loader.load_json(p) for p in paths.list_json_files(paths.BLOCKS_DIR)]

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(paths.BLOCKS_DIR) or {
            "name": "",
            "properties": {},
            "texture": {"type": "cuboid", "path": ""},
            "sounds": {"break": "", "place": ""},
        }

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        block = Block.from_dict(data)
        self._field_name.value = block.name
        p = block.properties
        self._field_roughness.value = p.roughness
        self._field_roughness_value.value = f"{p.roughness:.2f}"
        for k, sw in self._bool_switches.items():
            sw.value = bool(getattr(p, k))
        self._seg_type.selected = [block.texture.type]
        self._field_tex_path.value = block.texture.path
        self._field_normal.value = block.texture.normal or ""
        self._field_normal.disabled = (block.texture.type == "cross")
        self._field_break.value = block.sounds.break_
        self._field_place.value = block.sounds.place
        self._field_walk.value = block.sounds.walk or ""

        return ft.Column(
            [
                self._field_name,
                ft.Container(
                    ft.Column(
                        [
                            ft.Text("Properties", weight=ft.FontWeight.BOLD),
                            ft.Container(
                                ft.Row([sw for sw in self._bool_switches.values()], wrap=True),
                            ),
                            ft.Text("roughness"),
                            ft.Row([self._field_roughness, self._field_roughness_value]),
                        ]
                    ),
                    padding=10,
                    border_radius=12,
                    bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
                ),
                ft.Container(
                    ft.Column(
                        [
                            ft.Text("Texture", weight=ft.FontWeight.BOLD),
                            self._seg_type,
                            self._field_tex_path,
                            self._field_normal,
                        ]
                    ),
                    padding=10,
                    border_radius=12,
                    bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
                ),
                ft.Container(
                    ft.Column(
                        [
                            ft.Text("Sounds", weight=ft.FontWeight.BOLD),
                            ft.Row([self._field_break, self._field_place]),
                            self._field_walk,
                        ]
                    ),
                    padding=10,
                    border_radius=12,
                    bgcolor=ft.Colors.SURFACE_CONTAINER_HIGH,
                ),
            ],
            scroll=ft.ScrollMode.AUTO,
            expand=True,
        )

    def form_to_data(self) -> dict[str, Any] | None:
        block = Block(
            name=self._field_name.value or "",
            properties=BlockProperties(
                **{k: bool(sw.value) for k, sw in self._bool_switches.items()},
                roughness=float(self._field_roughness.value or 0.0),
            ),
            texture=Texture(
                type=(self._seg_type.selected or ["cuboid"])[0],
                path=self._field_tex_path.value or "",
                normal=self._field_normal.value or None,
            ),
            sounds=Sounds(
                break_=self._field_break.value or "",
                place=self._field_place.value or "",
                walk=self._field_walk.value or None,
            ),
        )
        return block.to_dict()

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.validate_block(Block.from_dict(data))

    def save_data(self, data: dict[str, Any]) -> str:
        block = Block.from_dict(data)
        # Sync registry before saving so id allocation is stable.
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

    # --- events -----------------------------------------------------------

    def _on_rough_change(self, e: ft.ControlEvent) -> None:
        val = float(e.control.value or 0)
        self._field_roughness_value.value = f"{val:.2f}"
        self._field_roughness_value.update()