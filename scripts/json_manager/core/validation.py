"""Schema-driven validation for managed resources.

``validate(schema, data)`` checks required / range / per-field validators
then cross-validators. Per-resource wrappers keep the public API the
views used before the refactor.
"""

from __future__ import annotations

from dataclasses import dataclass

from .schema import (
    BLOCK_SCHEMA,
    CREATURE_SCHEMA,
    ITEM_SCHEMA,
    Schema,
    get_path,
)


@dataclass
class ValidationResult:
    ok: bool
    errors: list[str]

    @classmethod
    def ok_result(cls) -> ValidationResult:
        return cls(True, [])

    @classmethod
    def fail(cls, *errors: str) -> ValidationResult:
        return cls(False, list(errors))


def validate(schema: Schema, data: dict) -> ValidationResult:
    errors: list[str] = []
    for f in schema.fields:
        v = get_path(data, f.key, f.default)
        if f.required and (v is None or (isinstance(v, str) and not v.strip())):
            errors.append(f"{f.label_text} must not be empty")
        if f.range is not None and isinstance(v, (int, float)):
            if not (f.range[0] <= v <= f.range[1]):
                errors.append(f"{f.label_text} must be in [{f.range[0]}, {f.range[1]}]")
        for val in f.validators:
            try:
                msg = val(v, data)
            except Exception as ex:  # noqa: BLE001
                msg = str(ex)
            if msg:
                errors.append(msg)
    for cv in schema.cross_validators:
        try:
            msg = cv(data)
        except Exception as ex:  # noqa: BLE001
            msg = str(ex)
        if msg:
            errors.append(msg)
    return ValidationResult(not errors, errors)


def validate_block(data: dict) -> ValidationResult:
    return validate(BLOCK_SCHEMA, data)


def validate_item(data: dict) -> ValidationResult:
    return validate(ITEM_SCHEMA, data)


def validate_creature(data: dict) -> ValidationResult:
    return validate(CREATURE_SCHEMA, data)
