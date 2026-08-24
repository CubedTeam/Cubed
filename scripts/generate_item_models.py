"""Generate embedded GLB models for item definitions with Blender.

Run from the repository root:

    blender --background --factory-startup \
        --python scripts/generate_item_models.py -- --assets-root assets
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from dataclasses import dataclass, field
from pathlib import Path, PurePosixPath
from typing import Any

import bpy


MODEL_SIZE = 0.5
SPRITE_THICKNESS = 0.03125
FACE_TEXTURES = ("front", "right", "back", "left", "top", "base")


class GenerationError(RuntimeError):
    """Raised when an item cannot be converted into a model."""


class SkipItem(RuntimeError):
    """Raised when an item intentionally has no visible model."""


@dataclass(frozen=True)
class ResourceLocation:
    namespace: str
    path: PurePosixPath

    @classmethod
    def parse(cls, value: Any, default_namespace: str) -> ResourceLocation:
        if not isinstance(value, str) or not value:
            raise GenerationError(f"invalid resource location: {value!r}")

        if value.count(":") > 1:
            raise GenerationError(f"invalid resource location: {value!r}")

        if ":" in value:
            namespace, raw_path = value.split(":", 1)
        else:
            namespace, raw_path = default_namespace, value

        path = PurePosixPath(raw_path)
        if (
            not namespace
            or not raw_path
            or path.is_absolute()
            or ".." in path.parts
            or "." in path.parts
        ):
            raise GenerationError(f"unsafe resource location: {value!r}")
        return cls(namespace=namespace, path=path)

    def filesystem_path(self, assets_root: Path) -> Path:
        return assets_root / self.namespace / Path(*self.path.parts)


@dataclass
class Report:
    entries: dict[str, list[str]] = field(
        default_factory=lambda: {
            "generated": [],
            "existing": [],
            "skipped": [],
            "failed": [],
        }
    )

    def add(self, status: str, detail: str) -> None:
        self.entries[status].append(detail)
        print(f"[{status.upper()}] {detail}")

    def print_summary(self) -> None:
        counts = " ".join(
            f"{status}={len(details)}" for status, details in self.entries.items()
        )
        print(f"\nSummary: {counts}")


def read_json(path: Path) -> dict[str, Any]:
    try:
        with path.open("r", encoding="utf-8") as stream:
            value = json.load(stream)
    except (OSError, json.JSONDecodeError) as exc:
        raise GenerationError(f"cannot read {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise GenerationError(f"JSON root must be an object: {path}")
    return value


def clear_scene() -> None:
    if bpy.context.object is not None and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    for collection in (bpy.data.meshes, bpy.data.materials, bpy.data.images):
        for block in list(collection):
            if block.users == 0:
                collection.remove(block)


def create_material(name: str, texture_path: Path):
    if not texture_path.is_file():
        raise GenerationError(f"texture does not exist: {texture_path}")

    try:
        image = bpy.data.images.load(str(texture_path.resolve()), check_existing=False)
    except RuntimeError as exc:
        raise GenerationError(f"cannot load texture {texture_path}: {exc}") from exc

    image.name = f"{name}_texture"
    image.pack()

    material = bpy.data.materials.new(name=name)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    shader = nodes.get("Principled BSDF")
    if shader is None:
        raise GenerationError("Blender did not create a Principled BSDF node")

    shader.inputs["Metallic"].default_value = 0.0
    shader.inputs["Roughness"].default_value = 1.0
    texture = nodes.new("ShaderNodeTexImage")
    texture.image = image
    texture.interpolation = "Closest"
    links.new(texture.outputs["Color"], shader.inputs["Base Color"])
    links.new(texture.outputs["Alpha"], shader.inputs["Alpha"])
    return material, image


def create_mesh_object(
    name: str,
    vertices: list[tuple[float, float, float]],
    faces: list[tuple[int, int, int, int]],
    face_uvs: list[tuple[tuple[float, float], ...]],
    materials: list[Any],
    material_indices: list[int],
):
    mesh = bpy.data.meshes.new(f"{name}_mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.materials.clear()
    for material in materials:
        mesh.materials.append(material)

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon, uvs, material_index in zip(
        mesh.polygons, face_uvs, material_indices, strict=True
    ):
        polygon.material_index = material_index
        polygon.use_smooth = False
        for loop_index, uv in zip(polygon.loop_indices, uvs, strict=True):
            uv_layer.data[loop_index].uv = uv

    mesh.validate(verbose=True)
    mesh.update(calc_edges=True)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    return obj


def append_quad(
    vertices: list[tuple[float, float, float]],
    faces: list[tuple[int, int, int, int]],
    face_uvs: list[tuple[tuple[float, float], ...]],
    coords: tuple[tuple[float, float, float], ...],
    uvs: tuple[tuple[float, float], ...],
) -> None:
    start = len(vertices)
    # AI-generated: Convert game Y-up coordinates to Blender Z-up coordinates.
    vertices.extend((x, -z, y) for x, y, z in coords)
    faces.append((start, start + 1, start + 2, start + 3))
    face_uvs.append(uvs)


def build_cube(name: str, texture_dir: Path):
    texture_paths = [texture_dir / f"{face}.png" for face in FACE_TEXTURES]
    missing = [str(path) for path in texture_paths if not path.is_file()]
    if missing:
        raise GenerationError(f"missing cube textures: {', '.join(missing)}")

    half = MODEL_SIZE / 2.0
    cube_faces = (
        (
            (-half, -half, half),
            (half, -half, half),
            (half, half, half),
            (-half, half, half),
        ),
        (
            (half, -half, half),
            (half, -half, -half),
            (half, half, -half),
            (half, half, half),
        ),
        (
            (half, -half, -half),
            (-half, -half, -half),
            (-half, half, -half),
            (half, half, -half),
        ),
        (
            (-half, -half, -half),
            (-half, -half, half),
            (-half, half, half),
            (-half, half, -half),
        ),
        (
            (-half, half, half),
            (half, half, half),
            (half, half, -half),
            (-half, half, -half),
        ),
        (
            (-half, -half, -half),
            (half, -half, -half),
            (half, -half, half),
            (-half, -half, half),
        ),
    )
    standard_uv = ((0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0))

    vertices: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int, int]] = []
    face_uvs: list[tuple[tuple[float, float], ...]] = []
    materials = []
    images = []
    for face_name, texture_path, coords in zip(
        FACE_TEXTURES, texture_paths, cube_faces, strict=True
    ):
        material, image = create_material(f"{name}_{face_name}", texture_path)
        materials.append(material)
        images.append(image)
        append_quad(vertices, faces, face_uvs, coords, standard_uv)

    obj = create_mesh_object(
        name,
        vertices,
        faces,
        face_uvs,
        materials,
        list(range(len(cube_faces))),
    )
    return obj, images


def image_alpha(image) -> tuple[int, int, list[list[bool]]]:
    width, height = (int(value) for value in image.size)
    if width <= 0 or height <= 0:
        raise GenerationError(f"texture has invalid dimensions: {image.filepath}")

    pixels = tuple(image.pixels)
    opaque = [
        [pixels[(y * width + x) * 4 + 3] > 0.0 for x in range(width)]
        for y in range(height)
    ]
    return width, height, opaque


def build_sprite(name: str, texture_path: Path):
    material, image = create_material(f"{name}_sprite", texture_path)
    width, height, opaque = image_alpha(image)

    visible = [(x, y) for y in range(height) for x in range(width) if opaque[y][x]]
    if not visible:
        raise SkipItem("texture has no visible pixels")

    min_x = min(x for x, _ in visible)
    max_x = max(x for x, _ in visible)
    min_y = min(y for _, y in visible)
    max_y = max(y for _, y in visible)
    visible_width = max_x - min_x + 1
    visible_height = max_y - min_y + 1
    pixel_size = MODEL_SIZE / max(visible_width, visible_height)
    model_width = visible_width * pixel_size
    model_height = visible_height * pixel_size
    half_depth = SPRITE_THICKNESS / 2.0

    vertices: list[tuple[float, float, float]] = []
    faces: list[tuple[int, int, int, int]] = []
    face_uvs: list[tuple[tuple[float, float], ...]] = []

    def is_opaque(x: int, y: int) -> bool:
        return 0 <= x < width and 0 <= y < height and opaque[y][x]

    # AI-generated: Build only visible sprite surfaces and exposed outline edges.
    for x, y in visible:
        x0 = (x - min_x) * pixel_size - model_width / 2.0
        x1 = x0 + pixel_size
        y0 = (y - min_y) * pixel_size - model_height / 2.0
        y1 = y0 + pixel_size
        u0, u1 = x / width, (x + 1) / width
        v0, v1 = y / height, (y + 1) / height
        center_uv = ((x + 0.5) / width, (y + 0.5) / height)
        side_uv = (center_uv, center_uv, center_uv, center_uv)

        append_quad(
            vertices,
            faces,
            face_uvs,
            (
                (x0, y0, half_depth),
                (x1, y0, half_depth),
                (x1, y1, half_depth),
                (x0, y1, half_depth),
            ),
            ((u0, v0), (u1, v0), (u1, v1), (u0, v1)),
        )
        append_quad(
            vertices,
            faces,
            face_uvs,
            (
                (x1, y0, -half_depth),
                (x0, y0, -half_depth),
                (x0, y1, -half_depth),
                (x1, y1, -half_depth),
            ),
            ((u1, v0), (u0, v0), (u0, v1), (u1, v1)),
        )

        if not is_opaque(x - 1, y):
            append_quad(
                vertices,
                faces,
                face_uvs,
                (
                    (x0, y0, -half_depth),
                    (x0, y0, half_depth),
                    (x0, y1, half_depth),
                    (x0, y1, -half_depth),
                ),
                side_uv,
            )
        if not is_opaque(x + 1, y):
            append_quad(
                vertices,
                faces,
                face_uvs,
                (
                    (x1, y0, half_depth),
                    (x1, y0, -half_depth),
                    (x1, y1, -half_depth),
                    (x1, y1, half_depth),
                ),
                side_uv,
            )
        if not is_opaque(x, y - 1):
            append_quad(
                vertices,
                faces,
                face_uvs,
                (
                    (x0, y0, -half_depth),
                    (x1, y0, -half_depth),
                    (x1, y0, half_depth),
                    (x0, y0, half_depth),
                ),
                side_uv,
            )
        if not is_opaque(x, y + 1):
            append_quad(
                vertices,
                faces,
                face_uvs,
                (
                    (x0, y1, half_depth),
                    (x1, y1, half_depth),
                    (x1, y1, -half_depth),
                    (x0, y1, -half_depth),
                ),
                side_uv,
            )

    obj = create_mesh_object(
        name,
        vertices,
        faces,
        face_uvs,
        [material],
        [0] * len(faces),
    )
    return obj, [image]


def export_glb(obj, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = output_path.with_name(f".{output_path.stem}.tmp.glb")
    if temporary_path.exists():
        temporary_path.unlink()

    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    try:
        result = bpy.ops.export_scene.gltf(
            filepath=str(temporary_path.resolve()),
            export_format="GLB",
            use_selection=True,
            export_yup=True,
            export_apply=True,
            export_materials="EXPORT",
        )
        if "FINISHED" not in result or not temporary_path.is_file():
            raise GenerationError(f"Blender did not export {output_path}")
        os.replace(temporary_path, output_path)
    except Exception:
        if temporary_path.exists():
            temporary_path.unlink()
        raise


def block_texture_source(
    assets_root: Path,
    item: dict[str, Any],
    item_namespace: str,
) -> tuple[str, Path]:
    block_location = ResourceLocation.parse(item.get("block"), item_namespace)
    block_json_path = (
        assets_root
        / block_location.namespace
        / "blocks"
        / Path(*block_location.path.parts)
    ).with_suffix(".json")
    block = read_json(block_json_path)
    texture = block.get("texture")
    if not isinstance(texture, dict):
        properties = block.get("properties")
        if isinstance(properties, dict) and properties.get("is_gas") is True:
            raise SkipItem("block has no visible geometry")
        raise GenerationError(f"block has no texture object: {block_json_path}")

    texture_type = texture.get("type")
    texture_location = ResourceLocation.parse(
        texture.get("path"), block_location.namespace
    )
    texture_path = texture_location.filesystem_path(assets_root)
    if texture_type == "cuboid":
        return texture_type, texture_path
    if texture_type == "cross":
        return texture_type, texture_path / "cross.png"
    raise GenerationError(
        f"unsupported block texture type {texture_type!r}: {block_json_path}"
    )


def generate_item(
    assets_root: Path, item_path: Path, namespace: str
) -> tuple[str, str]:
    item = read_json(item_path)
    item_location = ResourceLocation.parse(item.get("name"), namespace)
    output_path = (
        assets_root
        / item_location.namespace
        / "models"
        / "items"
        / Path(*item_location.path.parts)
    ).with_suffix(".glb")
    display_path = output_path.relative_to(assets_root.parent).as_posix()
    if output_path.exists():
        return "existing", display_path

    clear_scene()
    item_type = item.get("type")
    if item_type == "block":
        texture_type, texture_path = block_texture_source(
            assets_root, item, item_location.namespace
        )
        if texture_type == "cuboid":
            obj, _images = build_cube(item_location.path.name, texture_path)
        else:
            obj, _images = build_sprite(item_location.path.name, texture_path)
    else:
        texture_location = ResourceLocation.parse(
            item.get("texture"), item_location.namespace
        )
        obj, _images = build_sprite(
            item_location.path.name,
            texture_location.filesystem_path(assets_root),
        )

    export_glb(obj, output_path)
    return "generated", display_path


def discover_items(assets_root: Path) -> list[tuple[Path, str]]:
    discovered: list[tuple[Path, str]] = []
    if not assets_root.is_dir():
        raise GenerationError(f"assets root does not exist: {assets_root}")

    for namespace_dir in sorted(
        path for path in assets_root.iterdir() if path.is_dir()
    ):
        items_dir = namespace_dir / "items"
        if not items_dir.is_dir():
            continue
        discovered.extend(
            (path, namespace_dir.name) for path in sorted(items_dir.rglob("*.json"))
        )
    return discovered


def parse_args() -> argparse.Namespace:
    script_args = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    default_assets = Path(__file__).resolve().parent.parent / "assets"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--assets-root",
        type=Path,
        default=default_assets,
        help="Root directory containing namespace folders (default: repository assets)",
    )
    return parser.parse_args(script_args)


def main() -> int:
    args = parse_args()
    assets_root = args.assets_root.expanduser().resolve()
    report = Report()

    try:
        items = discover_items(assets_root)
    except GenerationError as exc:
        print(f"[FAILED] {exc}", file=sys.stderr)
        return 1

    if not items:
        print(
            f"[FAILED] no item definitions found under {assets_root}", file=sys.stderr
        )
        return 1

    for item_path, namespace in items:
        source = item_path.relative_to(assets_root.parent).as_posix()
        try:
            status, detail = generate_item(assets_root, item_path, namespace)
            report.add(status, detail)
        except SkipItem as exc:
            report.add("skipped", f"{source}: {exc}")
        except Exception as exc:
            report.add("failed", f"{source}: {exc}")

    clear_scene()
    report.print_summary()
    return 1 if report.entries["failed"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
