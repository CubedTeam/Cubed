"""Schema-aware helpers for JSON fields managed outside static schemas."""

from __future__ import annotations

import json
import math
from copy import deepcopy
from dataclasses import dataclass, field
from typing import Any

from .schema import Schema

TYPE_STRING = "string"
TYPE_INTEGER = "integer"
TYPE_NUMBER = "number"
TYPE_BOOLEAN = "boolean"
TYPE_NULL = "null"
TYPE_OBJECT = "object"
TYPE_ARRAY = "array"

FIELD_TYPES = (
    TYPE_STRING,
    TYPE_INTEGER,
    TYPE_NUMBER,
    TYPE_BOOLEAN,
    TYPE_NULL,
    TYPE_OBJECT,
    TYPE_ARRAY,
)

_MISSING = object()


class CustomFieldError(ValueError):
    def __init__(self, code: str, *, path: str = "") -> None:
        super().__init__(code)
        self.code = code
        self.path = path


@dataclass
class _SchemaNode:
    terminal: bool = False
    children: dict[str, _SchemaNode] = field(default_factory=dict)


def _schema_tree(schema: Schema) -> _SchemaNode:
    root = _SchemaNode()
    for spec in schema.fields:
        node = root
        for part in spec.key.split("."):
            node = node.children.setdefault(part, _SchemaNode())
        node.terminal = True
    return root


def extract_extra_fields(schema: Schema, data: dict[str, Any]) -> dict[str, Any]:
    """Return a deep copy of values not owned by ``schema``."""

    # AI-generated: schema container nodes stay reserved even when malformed.
    def walk(value: Any, node: _SchemaNode) -> Any:
        if node.terminal:
            return _MISSING
        if not isinstance(value, dict):
            return _MISSING if node.children else deepcopy(value)

        result: dict[str, Any] = {}
        for key, child_value in value.items():
            child_node = node.children.get(key)
            if child_node is None:
                result[key] = deepcopy(child_value)
                continue
            extra = walk(child_value, child_node)
            if extra is not _MISSING:
                result[key] = extra
        return result if result else _MISSING

    extracted = walk(data, _schema_tree(schema))
    return extracted if isinstance(extracted, dict) else {}


def merge_known_with_extra(
    known: dict[str, Any], extra: dict[str, Any]
) -> dict[str, Any]:
    """Deep-merge JSON objects while keeping schema-owned values authoritative."""

    result = deepcopy(known)
    for key, value in extra.items():
        if key not in result:
            result[key] = deepcopy(value)
        elif isinstance(result[key], dict) and isinstance(value, dict):
            result[key] = merge_known_with_extra(result[key], value)
    return result


def normalize_path(path: str) -> str:
    raw = path.strip()
    if not raw:
        raise CustomFieldError("path_empty")
    parts = [part.strip() for part in raw.split(".")]
    if any(not part for part in parts):
        raise CustomFieldError("path_segment_empty", path=raw)
    return ".".join(parts)


def validate_new_path(schema: Schema, extra: dict[str, Any], path: str) -> str:
    normalized = normalize_path(path)
    parts = normalized.split(".")

    node = _schema_tree(schema)
    for index, part in enumerate(parts):
        if node.terminal:
            raise CustomFieldError("schema_conflict", path=normalized)
        child = node.children.get(part)
        if child is None:
            break
        if index == len(parts) - 1:
            raise CustomFieldError("schema_conflict", path=normalized)
        node = child

    current: Any = extra
    for index, part in enumerate(parts):
        if not isinstance(current, dict):
            raise CustomFieldError("parent_not_object", path=normalized)
        if index == len(parts) - 1:
            if part in current:
                raise CustomFieldError("field_exists", path=normalized)
            return normalized
        if part not in current:
            return normalized
        current = current[part]
    return normalized


def set_custom_path(
    schema: Schema, extra: dict[str, Any], path: str, value: Any
) -> str:
    normalized = validate_new_path(schema, extra, path)
    parts = normalized.split(".")
    current = extra
    for part in parts[:-1]:
        current = current.setdefault(part, {})
    current[parts[-1]] = deepcopy(value)
    return normalized


def delete_custom_path(extra: dict[str, Any], path: str) -> bool:
    parts = normalize_path(path).split(".")

    def remove(current: dict[str, Any], index: int) -> bool:
        part = parts[index]
        if part not in current:
            return False
        if index == len(parts) - 1:
            del current[part]
            return True
        child = current[part]
        if not isinstance(child, dict) or not remove(child, index + 1):
            return False
        if not child:
            del current[part]
        return True

    return remove(extra, 0)


def set_existing_path(extra: dict[str, Any], path: str, value: Any) -> None:
    parts = normalize_path(path).split(".")
    current: Any = extra
    for part in parts[:-1]:
        if not isinstance(current, dict) or part not in current:
            raise CustomFieldError("field_missing", path=path)
        current = current[part]
    if not isinstance(current, dict) or parts[-1] not in current:
        raise CustomFieldError("field_missing", path=path)
    current[parts[-1]] = deepcopy(value)


def field_type(value: Any) -> str:
    if value is None:
        return TYPE_NULL
    if isinstance(value, bool):
        return TYPE_BOOLEAN
    if isinstance(value, int):
        return TYPE_INTEGER
    if isinstance(value, float):
        return TYPE_NUMBER
    if isinstance(value, dict):
        return TYPE_OBJECT
    if isinstance(value, list):
        return TYPE_ARRAY
    return TYPE_STRING


def default_value(kind: str) -> Any:
    defaults = {
        TYPE_STRING: "",
        TYPE_INTEGER: 0,
        TYPE_NUMBER: 0.0,
        TYPE_BOOLEAN: False,
        TYPE_NULL: None,
        TYPE_OBJECT: {},
        TYPE_ARRAY: [],
    }
    if kind not in defaults:
        raise CustomFieldError("unknown_type")
    return deepcopy(defaults[kind])


def parse_field_value(kind: str, raw: Any) -> Any:
    if kind == TYPE_STRING:
        return "" if raw is None else str(raw)
    if kind == TYPE_BOOLEAN:
        if isinstance(raw, bool):
            return raw
        raise CustomFieldError("invalid_boolean")
    if kind == TYPE_NULL:
        return None
    if kind == TYPE_INTEGER:
        value = _parse_json_scalar(raw, "invalid_integer")
        if isinstance(value, bool) or not isinstance(value, int):
            raise CustomFieldError("invalid_integer")
        return value
    if kind == TYPE_NUMBER:
        value = _parse_json_scalar(raw, "invalid_number")
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise CustomFieldError("invalid_number")
        result = float(value)
        if not math.isfinite(result):
            raise CustomFieldError("invalid_number")
        return result
    if kind in (TYPE_OBJECT, TYPE_ARRAY):
        expected = dict if kind == TYPE_OBJECT else list
        code = "invalid_object" if kind == TYPE_OBJECT else "invalid_array"
        if isinstance(raw, expected):
            value = deepcopy(raw)
            _ensure_finite_numbers(value, code)
            return value
        try:
            value = json.loads(str(raw), parse_constant=_reject_json_constant)
        except (TypeError, ValueError, json.JSONDecodeError) as ex:
            raise CustomFieldError(code) from ex
        if not isinstance(value, expected):
            raise CustomFieldError(code)
        _ensure_finite_numbers(value, code)
        return value
    raise CustomFieldError("unknown_type")


def _parse_json_scalar(raw: Any, code: str) -> Any:
    if isinstance(raw, bool):
        raise CustomFieldError(code)
    if isinstance(raw, (int, float)):
        return raw
    try:
        return json.loads(str(raw), parse_constant=_reject_json_constant)
    except (TypeError, ValueError, json.JSONDecodeError) as ex:
        raise CustomFieldError(code) from ex


def _reject_json_constant(_value: str) -> None:
    raise ValueError


def _ensure_finite_numbers(value: Any, code: str) -> None:
    if isinstance(value, float) and not math.isfinite(value):
        raise CustomFieldError(code)
    if isinstance(value, dict):
        for child in value.values():
            _ensure_finite_numbers(child, code)
    elif isinstance(value, list):
        for child in value:
            _ensure_finite_numbers(child, code)
