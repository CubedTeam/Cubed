"""Data models for Cubed json resources.

Each @dataclass mirrors the on-disk json shape and provides to_dict /
from_dict helpers. Names match the existing schemas in assets/cubed/.
"""

from __future__ import annotations

from dataclasses import dataclass, field, asdict
from typing import Any


# --- Block -----------------------------------------------------------------


@dataclass
class BlockProperties:
    is_liquid: bool = False
    is_cross_plane: bool = False
    is_transparent: bool = False
    is_passable: bool = False
    is_discard: bool = False
    is_blend: bool = False
    is_transitional: bool = False
    is_gas: bool = False
    roughness: float = 0.75

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "BlockProperties":
        return cls(
            is_liquid=bool(data.get("is_liquid", False)),
            is_cross_plane=bool(data.get("is_cross_plane", False)),
            is_transparent=bool(data.get("is_transparent", False)),
            is_passable=bool(data.get("is_passable", False)),
            is_discard=bool(data.get("is_discard", False)),
            is_blend=bool(data.get("is_blend", False)),
            is_transitional=bool(data.get("is_transitional", False)),
            is_gas=bool(data.get("is_gas", False)),
            roughness=float(data.get("roughness", 0.75)),
        )


@dataclass
class Texture:
    type: str = "cuboid"
    path: str = ""
    normal: str | None = None

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "Texture":
        return cls(
            type=str(data.get("type", "cuboid")),
            path=str(data.get("path", "")),
            normal=data.get("normal"),
        )

    def to_dict(self) -> dict[str, Any]:
        out: dict[str, Any] = {"type": self.type, "path": self.path}
        if self.normal:
            out["normal"] = self.normal
        return out


@dataclass
class Sounds:
    break_: str = ""
    place: str = ""
    walk: str | None = None

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "Sounds":
        return cls(
            break_=str(data.get("break", "")),
            place=str(data.get("place", "")),
            walk=data.get("walk"),
        )

    def to_dict(self) -> dict[str, Any]:
        out: dict[str, Any] = {"break": self.break_, "place": self.place}
        if self.walk:
            out["walk"] = self.walk
        return out


@dataclass
class Block:
    name: str
    properties: BlockProperties = field(default_factory=BlockProperties)
    texture: Texture = field(default_factory=Texture)
    sounds: Sounds = field(default_factory=Sounds)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "Block":
        return cls(
            name=str(data.get("name", "")),
            properties=BlockProperties.from_dict(data.get("properties", {})),
            texture=Texture.from_dict(data.get("texture", {})),
            sounds=Sounds.from_dict(data.get("sounds", {})),
        )

    def to_dict(self) -> dict[str, Any]:
        out: dict[str, Any] = {
            "name": self.name,
            "properties": asdict(self.properties),
            "texture": self.texture.to_dict(),
        }
        # Water-like entries omit sounds entirely.
        if self.sounds.break_ or self.sounds.place or self.sounds.walk:
            out["sounds"] = self.sounds.to_dict()
        return out


# --- Item ------------------------------------------------------------------


@dataclass
class Item:
    name: str
    type: str = "block"
    block: str | None = None
    creature: str | None = None
    texture: str = ""
    description: str = ""

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "Item":
        return cls(
            name=str(data.get("name", "")),
            type=str(data.get("type", "block")),
            block=data.get("block"),
            creature=data.get("creature"),
            texture=str(data.get("texture", "")),
            description=str(data.get("description", "")),
        )

    def to_dict(self) -> dict[str, Any]:
        out: dict[str, Any] = {"name": self.name, "type": self.type}
        if self.type == "block" and self.block is not None:
            out["block"] = self.block
        elif self.type == "spawn_egg" and self.creature is not None:
            out["creature"] = self.creature
        out["texture"] = self.texture
        out["description"] = self.description
        return out


# --- Creature --------------------------------------------------------------


@dataclass
class Creature:
    name: str
    model: str = ""
    animation: str | None = None
    collision: str | None = None

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "Creature":
        return cls(
            name=str(data.get("name", "")),
            model=str(data.get("model", "")),
            animation=data.get("animation"),
            collision=data.get("collision"),
        )

    def to_dict(self) -> dict[str, Any]:
        out: dict[str, Any] = {"name": self.name, "model": self.model}
        if self.animation:
            out["animation"] = self.animation
        if self.collision:
            out["collision"] = self.collision
        return out


# --- Registry --------------------------------------------------------------


@dataclass
class Registry:
    blocks: dict[str, int] = field(default_factory=dict)
    items: dict[str, int] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "Registry":
        return cls(
            blocks={str(k): int(v) for k, v in data.get("blocks", {}).items()},
            items={str(k): int(v) for k, v in data.get("items", {}).items()},
        )

    def to_dict(self) -> dict[str, Any]:
        return {
            "blocks": {k: self.blocks[k] for k in sorted(self.blocks)},
            "items": {k: self.items[k] for k in sorted(self.items)},
        }

    def next_block_id(self) -> int:
        return max(self.blocks.values(), default=-1) + 1

    def next_item_id(self) -> int:
        return max(self.items.values(), default=-1) + 1