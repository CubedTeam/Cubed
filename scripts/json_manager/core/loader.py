"""Load/save json resources with backup safety and registry sync.

All write paths go through ``save_json`` so the .bak contract holds and
indentation stays consistent with the existing project files.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from loguru import logger

from . import paths
from .backup import backup_file
from .models import Block, Creature, Item, Registry

JSON_INDENT = 4
JSON_ENSURE_ASCII = False


# --- Low level -------------------------------------------------------------


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as fh:
        return json.load(fh)


def save_json(path: Path, data: dict[str, Any]) -> None:
    backup_file(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as fh:
        json.dump(data, fh, indent=JSON_INDENT, ensure_ascii=JSON_ENSURE_ASCII)
        fh.write("\n")


def delete_json(path: Path) -> None:
    backup_file(path)
    try:
        path.unlink()
    except FileNotFoundError:
        logger.warning(f"Tried to delete missing file: {path}")


# --- Blocks ----------------------------------------------------------------


def load_blocks() -> list[Block]:
    return [
        Block.from_dict(load_json(p)) for p in paths.list_json_files(paths.BLOCKS_DIR)
    ]


def save_block(block: Block) -> Path:
    path = paths.BLOCKS_DIR / f"{block.name}.json"
    save_json(path, block.to_dict())
    return path


def delete_block(name: str) -> None:
    delete_json(paths.BLOCKS_DIR / f"{name}.json")


# --- Items -----------------------------------------------------------------


def load_items() -> list[Item]:
    return [
        Item.from_dict(load_json(p)) for p in paths.list_json_files(paths.ITEMS_DIR)
    ]


def save_item(item: Item) -> Path:
    path = paths.ITEMS_DIR / f"{item.name}.json"
    save_json(path, item.to_dict())
    return path


def delete_item(name: str) -> None:
    delete_json(paths.ITEMS_DIR / f"{name}.json")


# --- Creatures -------------------------------------------------------------


def load_creatures() -> list[Creature]:
    return [
        Creature.from_dict(load_json(p))
        for p in paths.list_json_files(paths.CREATURES_DIR)
    ]


def save_creature(creature: Creature) -> Path:
    path = paths.CREATURES_DIR / f"{creature.name}.json"
    save_json(path, creature.to_dict())
    return path


def delete_creature(name: str) -> None:
    delete_json(paths.CREATURES_DIR / f"{name}.json")


# --- Registry --------------------------------------------------------------


def load_registry() -> Registry:
    if not paths.REGISTRY_FILE.is_file():
        return Registry()
    return Registry.from_dict(load_json(paths.REGISTRY_FILE))


def save_registry(registry: Registry) -> Path:
    save_json(paths.REGISTRY_FILE, registry.to_dict())
    return paths.REGISTRY_FILE


def sync_registry_for_block(block: Block, registry: Registry) -> bool:
    """Ensure the block has an id; allocate one if missing. Returns True
    when the registry object was mutated."""
    if block.name in registry.blocks:
        return False
    registry.blocks[block.name] = registry.next_block_id()
    return True


def sync_registry_for_item(item: Item, registry: Registry) -> bool:
    if item.name in registry.items:
        return False
    registry.items[item.name] = registry.next_item_id()
    return True


def remove_block_from_registry(name: str, registry: Registry) -> bool:
    if name in registry.blocks:
        del registry.blocks[name]
        return True
    return False


def remove_item_from_registry(name: str, registry: Registry) -> bool:
    if name in registry.items:
        del registry.items[name]
        return True
    return False


# --- Lang & lexicon --------------------------------------------------------


def load_lang_files() -> dict[str, dict[str, str]]:
    out: dict[str, dict[str, str]] = {}
    for p in paths.list_json_files(paths.LANG_DIR):
        out[p.stem] = load_json(p)
    return out


def save_lang_file(locale: str, data: dict[str, str]) -> Path:
    path = paths.LANG_DIR / f"{locale}.json"
    save_json(path, dict(sorted(data.items())))
    return path


def load_lexicon() -> dict[str, Any]:
    if not paths.LEXICON_FILE.is_file():
        return {"lastUpdateDate": "", "words": []}
    return load_json(paths.LEXICON_FILE)


def save_lexicon(data: dict[str, Any]) -> Path:
    save_json(paths.LEXICON_FILE, data)
    return paths.LEXICON_FILE


# --- Template --------------------------------------------------------------


def load_first_as_template(directory: Path) -> dict[str, Any] | None:
    files = paths.list_json_files(directory)
    if not files:
        return None
    return load_json(files[0])
