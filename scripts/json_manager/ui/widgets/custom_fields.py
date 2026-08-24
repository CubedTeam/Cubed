"""Tree editor for JSON fields that are not declared in a resource schema."""

from __future__ import annotations

import json
from copy import deepcopy
from typing import Any

import flet as ft

from ...core import i18n
from ...core.custom_fields import (
    FIELD_TYPES,
    TYPE_ARRAY,
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_NULL,
    TYPE_NUMBER,
    TYPE_OBJECT,
    TYPE_STRING,
    CustomFieldError,
    default_value,
    delete_custom_path,
    extract_extra_fields,
    field_type,
    parse_field_value,
    set_custom_path,
    set_existing_path,
)
from ...core.schema import Schema
from ..dialogs import confirm, snack
from ..safe import safe_update

TYPE_WIDTH = 150
PATH_WIDTH = 220


class CustomFieldsEditor(ft.Column):
    def __init__(self, schema: Schema) -> None:
        super().__init__(spacing=10)
        self.schema = schema
        self._extra: dict[str, Any] = {}
        self._entries: dict[str, tuple[str, ft.Control]] = {}
        self.controls = self._build_controls()

    def set_data(self, data: dict[str, Any]) -> None:
        self._extra = extract_extra_fields(self.schema, data)
        self._rebuild(update=False)

    def get_data(self) -> dict[str, Any]:
        result = deepcopy(self._extra)
        for path, (kind, control) in self._entries.items():
            raw = getattr(control, "value", None)
            try:
                value = parse_field_value(kind, raw)
            except CustomFieldError as ex:
                raise CustomFieldError(ex.code, path=path) from ex
            set_existing_path(result, path, value)
        self._extra = deepcopy(result)
        return result

    def rebuild_labels(self) -> None:
        self._rebuild()

    def _build_controls(self) -> list[ft.Control]:
        self._entries.clear()
        header = ft.Row(
            [
                ft.Text(
                    i18n.t("custom.help"),
                    size=12,
                    color=ft.Colors.ON_SURFACE_VARIANT,
                    expand=True,
                ),
                ft.FilledTonalButton(
                    i18n.t("custom.add"),
                    icon=ft.Icons.ADD,
                    on_click=lambda e: self._show_add_dialog(e),
                ),
            ],
            vertical_alignment=ft.CrossAxisAlignment.CENTER,
        )
        if not self._extra:
            body: ft.Control = ft.Text(
                i18n.t("custom.empty"), color=ft.Colors.ON_SURFACE_VARIANT
            )
        else:
            body = ft.Column(self._build_tree(self._extra), spacing=8)
        return [header, body]

    def _build_tree(self, values: dict[str, Any], prefix: str = "") -> list[ft.Control]:
        controls: list[ft.Control] = []
        for key, value in values.items():
            path = f"{prefix}.{key}" if prefix else key
            if isinstance(value, dict):
                controls.append(self._object_control(path, value))
            else:
                controls.append(self._value_control(path, value))
        return controls

    def _object_control(self, path: str, value: dict[str, Any]) -> ft.Control:
        protected = self._is_schema_container(path)
        title_controls: list[ft.Control] = [
            ft.Text(path, tooltip=path, expand=True, weight=ft.FontWeight.W_500)
        ]
        if protected:
            title_controls.append(
                ft.Text(
                    i18n.t("custom.schema_container"),
                    size=12,
                    color=ft.Colors.ON_SURFACE_VARIANT,
                )
            )
        else:
            title_controls.extend(
                [
                    self._type_dropdown(path, TYPE_OBJECT),
                    self._delete_button(path),
                ]
            )
        title_controls.append(
            ft.IconButton(
                ft.Icons.ADD,
                tooltip=i18n.t("custom.add_child"),
                on_click=lambda e, p=path: self._show_add_dialog(e, p),
            )
        )

        children = self._build_tree(value, path)
        if not children:
            children = [ft.Text(i18n.t("custom.empty_object"), color=ft.Colors.OUTLINE)]
        return ft.ExpansionTile(
            title=ft.Row(
                title_controls,
                spacing=8,
                vertical_alignment=ft.CrossAxisAlignment.CENTER,
            ),
            controls=[ft.Container(ft.Column(children, spacing=8), padding=8)],
            expanded=True,
            maintain_state=True,
        )

    def _value_control(self, path: str, value: Any) -> ft.Control:
        kind = field_type(value)
        editor = self._editor_for_value(kind, value)
        self._entries[path] = (kind, editor)
        return ft.Container(
            ft.Row(
                [
                    ft.Text(path, width=PATH_WIDTH, tooltip=path),
                    self._type_dropdown(path, kind),
                    ft.Container(editor, expand=True),
                    self._delete_button(path),
                ],
                spacing=8,
                vertical_alignment=ft.CrossAxisAlignment.CENTER,
            ),
            padding=ft.Padding(left=8, right=0, top=4, bottom=4),
        )

    def _editor_for_value(self, kind: str, value: Any) -> ft.Control:
        if kind == TYPE_BOOLEAN:
            return ft.Switch(value=bool(value))
        if kind == TYPE_NULL:
            return ft.Text("null", italic=True, color=ft.Colors.ON_SURFACE_VARIANT)
        if kind == TYPE_ARRAY:
            return ft.TextField(
                value=json.dumps(value, indent=2, ensure_ascii=False),
                multiline=True,
                min_lines=2,
                max_lines=8,
                dense=True,
            )
        if kind == TYPE_NUMBER:
            text = repr(float(value))
        elif kind == TYPE_INTEGER:
            text = str(value)
        else:
            text = "" if value is None else str(value)
        return ft.TextField(value=text, dense=True)

    def _type_dropdown(self, path: str, kind: str) -> ft.Dropdown:
        return ft.Dropdown(
            value=kind,
            options=self._type_options(),
            dense=True,
            width=TYPE_WIDTH,
            on_select=lambda e, p=path, old=kind: self._on_type_change(e, p, old),
        )

    def _type_options(self) -> list[ft.DropdownOption]:
        return [
            ft.dropdown.Option(
                key=kind,
                text=i18n.t(f"custom.type.{kind}"),
            )
            for kind in FIELD_TYPES
        ]

    def _delete_button(self, path: str) -> ft.IconButton:
        return ft.IconButton(
            ft.Icons.DELETE_OUTLINE,
            tooltip=i18n.t("custom.delete"),
            on_click=lambda e, p=path: self._on_delete(e, p),
        )

    def _show_add_dialog(self, e: ft.ControlEvent, parent: str = "") -> None:
        page = e.page
        path_field = ft.TextField(
            label=i18n.t("custom.path"),
            value=f"{parent}." if parent else "",
            autofocus=True,
        )
        type_dropdown = ft.Dropdown(
            label=i18n.t("custom.type"),
            value=TYPE_STRING,
            options=self._type_options(),
            on_select=lambda event: refresh_value_editor(event.control.value),
        )
        value_holder = ft.Container(
            self._dialog_editor(TYPE_STRING), padding=ft.Padding(top=8)
        )
        error_text = ft.Text(color=ft.Colors.RED_700, visible=False)

        def refresh_value_editor(kind: str | None) -> None:
            value_holder.content = self._dialog_editor(kind or TYPE_STRING)
            safe_update(value_holder)

        def submit(event: ft.ControlEvent) -> None:
            kind = type_dropdown.value or TYPE_STRING
            raw = getattr(value_holder.content, "value", None)
            try:
                value = {} if kind == TYPE_OBJECT else parse_field_value(kind, raw)
                current = self.get_data()
                set_custom_path(self.schema, current, path_field.value or "", value)
            except CustomFieldError as ex:
                if not ex.path:
                    ex = CustomFieldError(ex.code, path=path_field.value or "")
                error_text.value = self._error_message(ex)
                error_text.visible = True
                safe_update(error_text)
                return
            self._extra = current
            try:
                event.page.pop_dialog()
            except Exception:
                pass
            self._rebuild()

        dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text(i18n.t("custom.add_title")),
            content=ft.Column(
                [path_field, type_dropdown, value_holder, error_text],
                spacing=8,
                tight=True,
            ),
            actions=[
                ft.TextButton(
                    i18n.t("action.cancel"),
                    on_click=lambda event: event.page.pop_dialog(),
                ),
                ft.FilledButton(i18n.t("action.ok"), on_click=submit),
            ],
            actions_alignment=ft.MainAxisAlignment.END,
        )
        path_field.on_submit = submit
        page.show_dialog(dialog)

    def _dialog_editor(self, kind: str) -> ft.Control:
        value = default_value(kind)
        if kind == TYPE_OBJECT:
            return ft.Text(i18n.t("custom.object_after_add"))
        return self._editor_for_value(kind, value)

    def _on_type_change(self, e: ft.ControlEvent, path: str, old_kind: str) -> None:
        new_kind = e.control.value or old_kind
        if new_kind == old_kind:
            return
        try:
            current = self.get_data()
            old_value = self._get_path(current, path)
        except CustomFieldError as ex:
            e.control.value = old_kind
            safe_update(e.control)
            snack(e.page, self._error_message(ex), "error")
            return

        def apply_change() -> None:
            set_existing_path(current, path, default_value(new_kind))
            self._extra = current
            self._rebuild()

        if isinstance(old_value, (dict, list)) and old_value:
            confirm(
                e.page,
                i18n.t("custom.change_type_title"),
                i18n.t("custom.change_type_body", path=path),
                apply_change,
                self._rebuild,
            )
            return
        apply_change()

    def _on_delete(self, e: ft.ControlEvent, path: str) -> None:
        try:
            current = self.get_data()
        except CustomFieldError as ex:
            snack(e.page, self._error_message(ex), "error")
            return
        if delete_custom_path(current, path):
            self._extra = current
            self._rebuild()

    def _is_schema_container(self, path: str) -> bool:
        prefix = f"{path}."
        return any(spec.key.startswith(prefix) for spec in self.schema.fields)

    def _error_message(self, error: CustomFieldError) -> str:
        return i18n.t(
            f"custom.error.{error.code}",
            default=error.code,
            path=error.path,
        )

    def _get_path(self, data: dict[str, Any], path: str) -> Any:
        current: Any = data
        for part in path.split("."):
            current = current[part]
        return current

    def _rebuild(self, update: bool = True) -> None:
        # AI-generated: rebuild keeps tree controls aligned with JSON value types.
        self.controls = self._build_controls()
        if update:
            safe_update(self)
