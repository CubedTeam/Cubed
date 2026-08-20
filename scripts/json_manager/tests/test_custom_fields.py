from __future__ import annotations

import json
import unittest
from pathlib import Path

from scripts.json_manager.core.custom_fields import (
    TYPE_ARRAY,
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_NULL,
    TYPE_NUMBER,
    TYPE_OBJECT,
    TYPE_STRING,
    CustomFieldError,
    delete_custom_path,
    extract_extra_fields,
    merge_known_with_extra,
    parse_field_value,
    set_custom_path,
    validate_new_path,
)
from scripts.json_manager.core.models import Block, Creature, Item
from scripts.json_manager.core.schema import BLOCK_SCHEMA, ITEM_SCHEMA
from scripts.json_manager.ui.widgets.custom_fields import CustomFieldsEditor

PROJECT_ROOT = Path(__file__).resolve().parents[3]


class CustomFieldDataTests(unittest.TestCase):
    def test_extracts_nested_and_top_level_unknown_fields(self) -> None:
        data = {
            "name": "stone",
            "properties": {
                "is_liquid": False,
                "hardness": 4,
                "metadata": {},
            },
            "tags": ["solid"],
            "optional": None,
        }

        self.assertEqual(
            extract_extra_fields(BLOCK_SCHEMA, data),
            {
                "properties": {"hardness": 4, "metadata": {}},
                "tags": ["solid"],
                "optional": None,
            },
        )

    def test_merge_preserves_known_values_and_custom_siblings(self) -> None:
        known = {"name": "stone", "properties": {"is_liquid": False}}
        extra = {
            "name": "wrong",
            "properties": {"is_liquid": True, "hardness": 4},
            "tags": [],
        }

        self.assertEqual(
            merge_known_with_extra(known, extra),
            {
                "name": "stone",
                "properties": {"is_liquid": False, "hardness": 4},
                "tags": [],
            },
        )

    def test_path_validation_and_delete(self) -> None:
        extra: dict = {"metadata": {"author": "Cubed"}, "scalar": 1}
        self.assertEqual(
            validate_new_path(BLOCK_SCHEMA, extra, " properties.hardness "),
            "properties.hardness",
        )
        set_custom_path(BLOCK_SCHEMA, extra, "metadata.version", 2)
        self.assertEqual(extra["metadata"]["version"], 2)
        self.assertTrue(delete_custom_path(extra, "metadata.author"))
        self.assertEqual(extra["metadata"], {"version": 2})

        for path, code in (
            ("", "path_empty"),
            ("metadata..tag", "path_segment_empty"),
            ("name", "schema_conflict"),
            ("name.child", "schema_conflict"),
            ("properties", "schema_conflict"),
            ("metadata", "field_exists"),
            ("scalar.child", "parent_not_object"),
        ):
            with self.subTest(path=path), self.assertRaises(CustomFieldError) as ctx:
                validate_new_path(BLOCK_SCHEMA, extra, path)
            self.assertEqual(ctx.exception.code, code)

    def test_parses_all_supported_json_types(self) -> None:
        self.assertEqual(parse_field_value(TYPE_STRING, 12), "12")
        self.assertEqual(parse_field_value(TYPE_INTEGER, "12"), 12)
        self.assertEqual(parse_field_value(TYPE_NUMBER, "1.25"), 1.25)
        self.assertIs(parse_field_value(TYPE_BOOLEAN, True), True)
        self.assertIsNone(parse_field_value(TYPE_NULL, "ignored"))
        self.assertEqual(parse_field_value(TYPE_OBJECT, '{"a": 1}'), {"a": 1})
        self.assertEqual(parse_field_value(TYPE_ARRAY, '[1, "x"]'), [1, "x"])

        for kind, raw in (
            (TYPE_INTEGER, "1.5"),
            (TYPE_NUMBER, "NaN"),
            (TYPE_OBJECT, "[]"),
            (TYPE_ARRAY, "{}"),
            (TYPE_ARRAY, "[1e400]"),
        ):
            with self.subTest(kind=kind), self.assertRaises(CustomFieldError):
                parse_field_value(kind, raw)


class ResourceModelRoundTripTests(unittest.TestCase):
    def test_creature_preserves_existing_sounds(self) -> None:
        path = PROJECT_ROOT / "assets/cubed/creatures/pig.json"
        data = json.loads(path.read_text(encoding="utf-8"))

        self.assertEqual(Creature.from_dict(data).to_dict(), data)

    def test_item_preserves_custom_stack_size(self) -> None:
        data = {
            "name": "stone",
            "type": "block",
            "block": "cubed:stone",
            "texture": "cubed:textures/items/block/stone.png",
            "description": "",
            "max_stack_size": 16,
        }

        item = Item.from_dict(data)
        self.assertEqual(item.extra_fields, {"max_stack_size": 16})
        self.assertEqual(item.to_dict(), data)

    def test_block_preserves_nested_custom_field(self) -> None:
        data = {
            "name": "custom",
            "properties": {"is_liquid": False, "hardness": 5},
            "texture": {"type": "cuboid", "path": "cubed:test"},
        }

        output = Block.from_dict(data).to_dict()
        self.assertEqual(output["properties"]["hardness"], 5)
        self.assertEqual(output["name"], "custom")

    def test_schema_fields_override_injected_extras(self) -> None:
        item = Item.from_dict(
            {
                "name": "stone",
                "type": "block",
                "block": "cubed:stone",
                "texture": "cubed:stone",
            }
        )
        item.extra_fields = {"name": "wrong", "max_stack_size": 32}

        output = item.to_dict()
        self.assertEqual(output["name"], "stone")
        self.assertEqual(output["max_stack_size"], 32)
        self.assertEqual(
            extract_extra_fields(ITEM_SCHEMA, output), {"max_stack_size": 32}
        )


class CustomFieldsEditorTests(unittest.TestCase):
    def test_editor_round_trips_tree_and_all_value_controls(self) -> None:
        data = {
            "name": "stone",
            "type": "block",
            "block": "cubed:stone",
            "texture": "cubed:stone",
            "description": "",
            "max_stack_size": 16,
            "enabled": True,
            "optional": None,
            "tags": ["solid"],
            "metadata": {"author": "Cubed"},
            "empty": {},
        }
        editor = CustomFieldsEditor(ITEM_SCHEMA)
        editor.set_data(data)

        self.assertEqual(editor.get_data(), extract_extra_fields(ITEM_SCHEMA, data))

    def test_editor_reports_invalid_array_path(self) -> None:
        editor = CustomFieldsEditor(ITEM_SCHEMA)
        editor.set_data({"tags": ["solid"]})
        _kind, control = editor._entries["tags"]
        control.value = "["

        with self.assertRaises(CustomFieldError) as ctx:
            editor.get_data()
        self.assertEqual(ctx.exception.code, "invalid_array")
        self.assertEqual(ctx.exception.path, "tags")


if __name__ == "__main__":
    unittest.main()
