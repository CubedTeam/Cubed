"""Data models for Cubed json resources.

The dataclasses act as typed attribute carriers; serialization is fully
driven by the schemas declared in :mod:`schema` (one FieldSpec per
field, no hand-written ``from_dict`` / ``to_dict``).
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any

from .custom_fields import extract_extra_fields, merge_known_with_extra
from .schema import (
    BLOCK_SCHEMA,
    CREATURE_SCHEMA,
    ITEM_SCHEMA,
    schema_from_dict,
    schema_to_dict,
)

# --- Block ----------------------------------------------------------------

# AI-generated: root models retain fields outside their static schemas.


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


@dataclass
class Texture:
    type: str = "cuboid"
    path: str = ""
    normal: str | None = None


@dataclass
class Sounds:
    break_: str = ""
    place: str = ""
    walk: str | None = None


@dataclass
class Block:
    name: str
    properties: BlockProperties = field(default_factory=BlockProperties)
    texture: Texture = field(default_factory=Texture)
    sounds: Sounds = field(default_factory=Sounds)
    extra_fields: dict[str, Any] = field(default_factory=dict, repr=False)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> Block:
        norm = schema_from_dict(BLOCK_SCHEMA, data)
        snd = norm["sounds"]
        obj = cls(
            name=norm["name"],
            properties=BlockProperties(**norm["properties"]),
            texture=Texture(**norm["texture"]),
            sounds=Sounds(
                break_=snd.get("break", ""),
                place=snd.get("place", ""),
                walk=snd.get("walk"),
            ),
            extra_fields=extract_extra_fields(BLOCK_SCHEMA, data),
        )
        return obj

    def to_dict(self) -> dict[str, Any]:
        return merge_known_with_extra(
            schema_to_dict(BLOCK_SCHEMA, self), self.extra_fields
        )


# --- Item -----------------------------------------------------------------


@dataclass
class Item:
    name: str
    type: str = "block"
    block: str | None = None
    creature: str | None = None
    texture: str = ""
    description: str = ""
    extra_fields: dict[str, Any] = field(default_factory=dict, repr=False)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> Item:
        return cls(
            **schema_from_dict(ITEM_SCHEMA, data),
            extra_fields=extract_extra_fields(ITEM_SCHEMA, data),
        )

    def to_dict(self) -> dict[str, Any]:
        return merge_known_with_extra(
            schema_to_dict(ITEM_SCHEMA, self), self.extra_fields
        )


# --- Creature -------------------------------------------------------------


@dataclass
class Creature:
    name: str
    model: str = ""
    animation: str | None = None
    collision: str | None = None
    extra_fields: dict[str, Any] = field(default_factory=dict, repr=False)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> Creature:
        norm = schema_from_dict(CREATURE_SCHEMA, data)
        return cls(
            **norm,
            extra_fields=extract_extra_fields(CREATURE_SCHEMA, data),
        )

    def to_dict(self) -> dict[str, Any]:
        return merge_known_with_extra(
            schema_to_dict(CREATURE_SCHEMA, self), self.extra_fields
        )


# --- Registry -------------------------------------------------------------


@dataclass
class Registry:
    blocks: dict[str, int] = field(default_factory=dict)
    items: dict[str, int] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> Registry:
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
