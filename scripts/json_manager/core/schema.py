"""Declarative resource schemas.

Each resource (block / item / creature) is described as an ordered list
of :class:`FieldSpec`. The same declaration drives:

* serialization (``schema_to_dict`` / ``schema_from_dict``),
* the form UI (``SchemaForm``),
* validation (``validation.validate``).

Adding a new field is a one-liner: append a ``FieldSpec`` to the schema
list and everything else picks it up automatically.

Field paths use dotted notation for nested keys, e.g. ``"properties.is_liquid"``
maps to ``block.properties.is_liquid`` on the model and to the nested
``{"properties": {"is_liquid": ...}}`` shape in JSON.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Callable

# Sentinel used to distinguish "missing" from an explicit ``None`` value.
_MISSING = object()


@dataclass
class FieldSpec:
    key: str
    py_type: type
    default: Any = None
    widget: str = "text"            # text / switch / slider / segmented
    label: str = ""                  # empty -> use last segment of key
    section: str = "基本"
    required: bool = False
    range: tuple[float, float] | None = None
    divisions: int | None = None
    options: list[str] | None = None
    multiline: bool = False
    # UI visibility; receives the JSON-shaped dict currently loaded.
    visible_when: Callable[[dict], bool] | None = None
    # Serialization gate; receives (value, snapshot_dict). True -> omit.
    omit_when: Callable[[Any, dict], bool] | None = None
    # Attribute path override (defaults to ``key``). Needed when a JSON
    # key is a Python keyword (e.g. ``sounds.break`` -> ``sounds.break_``).
    attr: str | None = None
    # Per-field validators: (value, data) -> error message or None.
    validators: list[Callable[[Any, dict], str | None]] = field(default_factory=list)

    @property
    def label_text(self) -> str:
        return self.label or self.key.rsplit(".", 1)[-1]

    @property
    def top_key(self) -> str:
        return self.key.split(".", 1)[0]


@dataclass
class Schema:
    fields: list[FieldSpec]
    # (data_dict) -> error message or None, applied after per-field checks.
    cross_validators: list[Callable[[dict], str | None]] = field(default_factory=list)
    # Top-level container key -> predicate(snapshot); True => skip the whole
    # group (used to drop ``sounds`` when all of its sub-fields are empty).
    group_omits: dict[str, Callable[[dict], bool]] = field(default_factory=dict)


# --- path helpers ---------------------------------------------------------


def get_path(d: dict | None, key: str, default: Any = _MISSING) -> Any:
    """Read a dotted key from a nested dict."""
    cur: Any = d
    for part in key.split("."):
        if not isinstance(cur, dict):
            return default if default is not _MISSING else None
        cur = cur.get(part, _MISSING)
        if cur is _MISSING:
            return default if default is not _MISSING else None
    return cur


def set_path(d: dict, key: str, value: Any) -> None:
    parts = key.split(".")
    cur = d
    for p in parts[:-1]:
        nxt = cur.get(p)
        if not isinstance(nxt, dict):
            nxt = {}
            cur[p] = nxt
        cur = nxt
    cur[parts[-1]] = value


def _coerce(py_type: type, raw: Any) -> Any:
    if raw is None:
        return None
    try:
        if py_type is bool:
            return bool(raw)
        if py_type is float:
            return float(raw)
        if py_type is int:
            return int(raw)
        if py_type is str:
            return str(raw)
    except (TypeError, ValueError):
        return raw
    return raw


def _get_obj_attr(obj: Any, f: FieldSpec) -> Any:
    cur: Any = obj
    for part in (f.attr or f.key).split("."):
        cur = getattr(cur, part, None)
        if cur is None:
            return None
    return cur


# --- schema-driven ser/des ------------------------------------------------


def schema_from_dict(schema: Schema, data: dict | None) -> dict:
    """Normalize a raw JSON dict against the schema (apply defaults/coerce)."""
    data = data or {}
    out: dict[str, Any] = {}
    for f in schema.fields:
        raw = get_path(data, f.key, _MISSING)
        if raw is _MISSING or raw is None:
            v = f.default
        else:
            v = _coerce(f.py_type, raw)
        set_path(out, f.key, v)
    return out


def schema_default(schema: Schema) -> dict:
    out: dict[str, Any] = {}
    for f in schema.fields:
        set_path(out, f.key, f.default)
    return out


def schema_to_dict(schema: Schema, obj: Any) -> dict[str, Any]:
    """Build the JSON dict from an object using the schema."""
    snapshot: dict[str, Any] = {}
    for f in schema.fields:
        v = _get_obj_attr(obj, f)
        snapshot[f.key] = v
    # Evaluate group omittance against snapshot (flat dotted keys).
    skip: set[str] = set()
    for top, pred in schema.group_omits.items():
        try:
            if pred(snapshot):
                skip.add(top)
        except Exception:
            pass
    out: dict[str, Any] = {}
    for f in schema.fields:
        if f.top_key in skip:
            continue
        v = snapshot[f.key]
        if f.omit_when is not None:
            try:
                if f.omit_when(v, snapshot):
                    continue
            except Exception:
                pass
        if v is None:
            continue
        set_path(out, f.key, v)
    return out


# --- schema declarations --------------------------------------------------


# Validation helpers (kept inline so schemas stay self-contained).

def _name_pattern_error(v: Any, _d: dict) -> str | None:
    name = str(v or "")
    if not name:
        return None
    if not name.replace("_", "").isalnum():
        return "name must only contain letters, digits and underscores"
    return None


def _block_uniqueness(data: dict) -> str | None:
    from . import paths
    from .loader import load_blocks

    name = data.get("name", "")
    if not name:
        return None
    existing = {b.name for b in load_blocks()}
    if name in existing and not (paths.BLOCKS_DIR / f"{name}.json").is_file():
        return f"block '{name}' already exists"
    return None


def _item_uniqueness(data: dict) -> str | None:
    from . import paths
    from .loader import load_items

    name = data.get("name", "")
    if not name:
        return None
    existing = {i.name for i in load_items()}
    if name in existing and not (paths.ITEMS_DIR / f"{name}.json").is_file():
        return f"item '{name}' already exists"
    return None


def _creature_uniqueness(data: dict) -> str | None:
    from . import paths
    from .loader import load_creatures

    name = data.get("name", "")
    if not name:
        return None
    existing = {c.name for c in load_creatures()}
    if name in existing and not (paths.CREATURES_DIR / f"{name}.json").is_file():
        return f"creature '{name}' already exists"
    return None


def _item_type_ref(data: dict) -> str | None:
    kind = data.get("type", "")
    if kind == "block" and not data.get("block"):
        return "item of type 'block' needs a block reference"
    if kind == "spawn_egg" and not data.get("creature"):
        return "item of type 'spawn_egg' needs a creature reference"
    return None


BLOCK_SCHEMA = Schema(
    fields=[
        FieldSpec("name", str, "", section="基本", required=True,
                  validators=[_name_pattern_error]),
        FieldSpec("properties.is_liquid", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_cross_plane", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_transparent", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_passable", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_discard", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_blend", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_transitional", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.is_gas", bool, False, widget="switch", section="Properties"),
        FieldSpec("properties.roughness", float, 0.75, widget="slider",
                  section="Properties", range=(0.0, 1.0), divisions=20),
        FieldSpec("texture.type", str, "cuboid", widget="segmented",
                  section="Texture", options=["cuboid", "cross"]),
        FieldSpec("texture.path", str, "", section="Texture", required=True),
        FieldSpec("texture.normal", str, None, section="Texture",
                  visible_when=lambda d: get_path(d, "texture.type") != "cross",
                  omit_when=lambda v, _s: not v),
        FieldSpec("sounds.break", str, "", section="Sounds", attr="sounds.break_"),
        FieldSpec("sounds.place", str, "", section="Sounds"),
        FieldSpec("sounds.walk", str, None, section="Sounds",
                  omit_when=lambda v, _s: not v),
    ],
    cross_validators=[_block_uniqueness],
    group_omits={
        "sounds": lambda s: not (
            s.get("sounds.break") or s.get("sounds.place") or s.get("sounds.walk")
        ),
    },
)


ITEM_SCHEMA = Schema(
    fields=[
        FieldSpec("name", str, "", section="基本", required=True,
                  validators=[_name_pattern_error]),
        FieldSpec("type", str, "block", widget="segmented",
                  section="Type", options=["block", "spawn_egg"]),
        FieldSpec("block", str, None, section="Block reference",
                  label="block (cubed:<name>)",
                  visible_when=lambda d: d.get("type") == "block",
                  omit_when=lambda v, s: s.get("type") != "block" or not v),
        FieldSpec("creature", str, None, section="Creature reference",
                  label="creature (cubed:<name>)",
                  visible_when=lambda d: d.get("type") == "spawn_egg",
                  omit_when=lambda v, s: s.get("type") != "spawn_egg" or not v),
        FieldSpec("texture", str, "", section="Texture & description", required=True),
        FieldSpec("description", str, "", section="Texture & description", multiline=True),
    ],
    cross_validators=[_item_uniqueness, _item_type_ref],
)


CREATURE_SCHEMA = Schema(
    fields=[
        FieldSpec("name", str, "", section="基本", required=True,
                  validators=[_name_pattern_error]),
        FieldSpec("model", str, "", section="Model", required=True),
        FieldSpec("animation", str, None, section="Optional references",
                  omit_when=lambda v, _s: not v),
        FieldSpec("collision", str, None, section="Optional references",
                  omit_when=lambda v, _s: not v),
    ],
    cross_validators=[_creature_uniqueness],
)