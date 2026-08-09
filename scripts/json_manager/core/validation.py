"""Lightweight validation for managed resources.

Validation errors are surfaced to the UI; the tool deliberately rejects
clearly malformed data rather than silently writing it.
"""

from __future__ import annotations

from dataclasses import dataclass

from .loader import load_blocks, load_creatures, load_items
from .models import Block, Creature, Item
from . import paths


@dataclass
class ValidationResult:
    ok: bool
    errors: list[str]

    @classmethod
    def ok_result(cls) -> "ValidationResult":
        return cls(True, [])

    @classmethod
    def fail(cls, *errors: str) -> "ValidationResult":
        return cls(False, list(errors))


def validate_block(block: Block) -> ValidationResult:
    if not block.name:
        return ValidationResult.fail("name must not be empty")
    if not block.name.replace("_", "").isalnum():
        return ValidationResult.fail(
            "name must only contain letters, digits and underscores"
        )
    existing = {b.name for b in load_blocks()}
    # Allow self-match when editing an existing block.
    if block.name in existing and not (paths.BLOCKS_DIR / f"{block.name}.json").exists():
        return ValidationResult.fail(f"block '{block.name}' already exists")
    if not block.texture.path:
        return ValidationResult.fail("texture.path must not be empty")
    if not 0.0 <= block.properties.roughness <= 1.0:
        return ValidationResult.fail("roughness must be in [0.0, 1.0]")
    return ValidationResult.ok_result()


def validate_item(item: Item) -> ValidationResult:
    if not item.name:
        return ValidationResult.fail("name must not be empty")
    existing = {i.name for i in load_items()}
    if item.name in existing and not (paths.ITEMS_DIR / f"{item.name}.json").exists():
        return ValidationResult.fail(f"item '{item.name}' already exists")
    if item.type == "block" and not item.block:
        return ValidationResult.fail("item of type 'block' needs a block reference")
    if item.type == "spawn_egg" and not item.creature:
        return ValidationResult.fail(
            "item of type 'spawn_egg' needs a creature reference"
        )
    if not item.texture:
        return ValidationResult.fail("texture must not be empty")
    return ValidationResult.ok_result()


def validate_creature(creature: Creature) -> ValidationResult:
    if not creature.name:
        return ValidationResult.fail("name must not be empty")
    existing = {c.name for c in load_creatures()}
    if creature.name in existing and not (
        paths.CREATURES_DIR / f"{creature.name}.json"
    ).exists():
        return ValidationResult.fail(f"creature '{creature.name}' already exists")
    if not creature.model:
        return ValidationResult.fail("model must not be empty")
    return ValidationResult.ok_result()