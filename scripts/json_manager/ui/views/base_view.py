"""Base view shared by blocks/items/creatures.

Layout (left to right):
    file list | divider | content pane (form or raw) | divider | action rail

The action rail lives on the right as a fixed-width column so buttons
keep a consistent size and vertical alignment across all views.
"""

from __future__ import annotations

from typing import Any

import flet as ft

from ...core import i18n, loader, validation
from ...core.custom_fields import CustomFieldError
from ...core.paths import list_json_files
from ..dialogs import confirm, snack
from ..safe import safe_update
from ..widgets.file_list import FileList
from ..widgets.raw_editor import RawEditor


class BaseResourceView(ft.Column):
    title: str = ""
    title_key: str | None = None
    directory = None  # set in subclass
    name_field_label = "name"

    def _t_title(self) -> str:
        if self.title_key:
            return i18n.t(self.title_key, default=self.title)
        return self.title

    def __init__(self, page: ft.Page) -> None:
        super().__init__()
        self.page_ctx = page
        self.expand = True
        self.spacing = 10
        self.selected_name: str | None = None
        self.current_data: dict[str, Any] = {}

        self.file_list = FileList(
            names=[],
            on_select=self._on_select_name,
            on_new=self._on_new,
            title=self._t_title(),
        )

        # Mode toggle: keep natural MD3 pill size, do NOT expand.
        self.mode_toggle = ft.SegmentedButton(
            selected=["form"],
            segments=[
                ft.Segment(
                    value="form",
                    label=ft.Text(i18n.t("view.base.mode_form")),
                    icon=ft.Icon(ft.Icons.EDIT_NOTE),
                ),
                ft.Segment(
                    value="raw",
                    label=ft.Text(i18n.t("view.base.mode_raw")),
                    icon=ft.Icon(ft.Icons.DATA_OBJECT),
                ),
            ],
            allow_multiple_selection=False,
            allow_empty_selection=False,
            on_change=self._on_mode_change,
        )

        # AI-generated: single content slot to avoid visible-toggle blank bugs.
        self.content_slot = ft.Container(expand=True)
        self.raw_editor = RawEditor()

        # Action buttons sit in the top toolbar on the right (natural size).
        self.action_row = ft.Row(
            [],
            alignment=ft.MainAxisAlignment.END,
            spacing=8,
            expand=False,
        )

        self.controls = [
            # Top toolbar: mode toggle (left) + actions (right).
            ft.Row(
                [self.mode_toggle, ft.Container(expand=True), self.action_row],
                alignment=ft.MainAxisAlignment.START,
                vertical_alignment=ft.CrossAxisAlignment.CENTER,
                spacing=8,
            ),
            ft.Row(
                [
                    ft.Container(self.file_list, width=240, expand=False),
                    ft.VerticalDivider(width=1),
                    self.content_slot,
                ],
                expand=True,
                spacing=0,
            ),
        ]
        self.refresh_list()
        # Initial empty-state render so toolbar buttons show up.
        self._render()

    # --- subclass hooks ---------------------------------------------------

    def load_all(self) -> list[dict[str, Any]]:
        return [loader.load_json(p) for p in list_json_files(self.directory)]

    def name_of(self, data: dict[str, Any]) -> str:
        return str(data.get("name", ""))

    def blank_form_data(self) -> dict[str, Any]:
        return loader.load_first_as_template(self.directory) or {"name": ""}

    def build_form(self, data: dict[str, Any]) -> ft.Control:
        raise NotImplementedError

    def form_to_data(self) -> dict[str, Any] | None:
        raise NotImplementedError

    def validate(self, data: dict[str, Any]) -> validation.ValidationResult:
        return validation.ValidationResult.ok_result()

    def save_data(self, data: dict[str, Any]) -> str:
        raise NotImplementedError

    def delete_data(self, name: str) -> None:
        raise NotImplementedError

    # --- mode switch ------------------------------------------------------

    def _on_mode_change(self, e: ft.ControlEvent) -> None:
        mode = (self.mode_toggle.selected or ["form"])[0]
        if mode == "raw":
            try:
                data = self.form_to_data()
            except CustomFieldError as ex:
                self._reject_mode_change("form", self._custom_error_message(ex))
                return
            if data is None:
                self._reject_mode_change("form")
                return
            self.current_data = data
        else:
            data = self._read_raw_payload()
            if data is None:
                self._reject_mode_change("raw")
                return
            self.current_data = data
        self._render(mode)

    def _reject_mode_change(self, mode: str, message: str | None = None) -> None:
        self.mode_toggle.selected = [mode]
        safe_update(self.mode_toggle)
        if message:
            snack(self.page_ctx, message, "error")

    def _read_raw_payload(self) -> dict[str, Any] | None:
        import json

        def reject_constant(value: str) -> None:
            raise ValueError(f"invalid JSON constant: {value}")

        try:
            data = json.loads(
                self.raw_editor.get_value(), parse_constant=reject_constant
            )
        except (json.JSONDecodeError, ValueError) as ex:
            snack(
                self.page_ctx,
                i18n.t("view.base.json_parse_failed", err=str(ex)),
                "error",
            )
            return None
        if not isinstance(data, dict):
            snack(self.page_ctx, i18n.t("view.base.json_root_object"), "error")
            return None
        return data

    # --- actions ----------------------------------------------------------

    def refresh_list(self) -> None:
        datas = self.load_all()
        self.names = [self.name_of(d) for d in datas]
        self.file_list.refresh(self.names)

    def _on_select_name(self, name: str) -> None:
        path = self.directory / f"{name}.json"
        if not path.is_file():
            return
        self.selected_name = name
        self.current_data = loader.load_json(path)
        self._render(self._mode())

    def _on_new(self) -> None:
        self.current_data = self.blank_form_data()
        existing = set(self.names)
        base = "new"
        i = 1
        candidate = base
        while candidate in existing:
            candidate = f"{base}_{i}"
            i += 1
        self.current_data["name"] = candidate
        self.selected_name = None
        self.mode_toggle.selected = ["form"]
        self._render("form")

    def _mode(self) -> str:
        return (self.mode_toggle.selected or ["form"])[0]

    def _render(self, mode: str | None = None) -> None:
        mode = mode or self._mode()
        if mode == "raw":
            self.raw_editor.set_value(_pretty(self.current_data))
            self.content_slot.content = self.raw_editor
        else:
            self.content_slot.content = self.build_form(self.current_data)
        self.action_row.controls = self._action_buttons()
        safe_update(self.content_slot)
        safe_update(self.action_row)

    def _action_buttons(self) -> list[ft.Control]:
        return [
            ft.FilledTonalButton(
                i18n.t("action.save"),
                icon=ft.Icons.SAVE_OUTLINED,
                on_click=self._on_save,
            ),
            ft.OutlinedButton(
                i18n.t("action.delete"),
                icon=ft.Icons.DELETE_OUTLINE,
                on_click=self._onDelete,
                disabled=self.selected_name is None,
            ),
            ft.OutlinedButton(
                i18n.t("action.duplicate"),
                icon=ft.Icons.CONTENT_COPY,
                on_click=self._onDuplicate,
                disabled=not self.current_data,
            ),
        ]

    def _current_payload(self) -> dict[str, Any] | None:
        if self._mode() == "raw":
            return self._read_raw_payload()
        try:
            return self.form_to_data()
        except CustomFieldError as ex:
            snack(self.page_ctx, self._custom_error_message(ex), "error")
            return None

    def _custom_error_message(self, error: CustomFieldError) -> str:
        return i18n.t(
            f"custom.error.{error.code}",
            default=error.code,
            path=error.path,
        )

    def _on_save(self, _e: ft.ControlEvent) -> None:
        data = self._current_payload()
        if data is None:
            return
        result = self.validate(data)
        if not result.ok:
            snack(self.page_ctx, "\n".join(result.errors), "error")
            return
        try:
            saved_name = self.save_data(data)
        except Exception as ex:  # noqa: BLE001
            snack(
                self.page_ctx,
                i18n.t("view.base.save_failed", err=str(ex)),
                "error",
            )
            return
        self.selected_name = saved_name
        self.current_data = data
        self.refresh_list()
        snack(self.page_ctx, i18n.t("view.base.saved", name=saved_name), "ok")

    def _onDelete(self, _e: ft.ControlEvent) -> None:
        if self.selected_name is None:
            return

        def _do_delete():
            self.delete_data(self.selected_name)
            snack(
                self.page_ctx,
                i18n.t("view.base.deleted", name=self.selected_name),
                "ok",
            )
            self.selected_name = None
            self.current_data = self.blank_form_data() or {}
            self.mode_toggle.selected = ["form"]
            self.refresh_list()
            self._render("form")

        confirm(
            self.page_ctx,
            i18n.t("view.base.confirm_delete_title"),
            i18n.t("view.base.confirm_delete_body", name=self.selected_name),
            _do_delete,
        )

    def _onDuplicate(self, _e: ft.ControlEvent) -> None:
        data = self._current_payload()
        if data is None:
            return
        existing = set(self.names)
        base = data.get("name", "copy") + "_copy"
        i = 1
        cand = base
        while cand in existing:
            cand = f"{base}_{i}"
            i += 1
        data["name"] = cand
        self.current_data = data
        self.selected_name = None
        self.mode_toggle.selected = ["form"]
        self._render("form")


def _pretty(data: dict[str, Any]) -> str:
    import json

    return json.dumps(data, indent=4, ensure_ascii=False)
