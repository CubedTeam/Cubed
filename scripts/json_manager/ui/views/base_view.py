"""Base view shared by blocks/items/creatures.

Provides the three-pane layout (file list + form/raw tabs + action bar)
and wires common actions (save, delete, duplicate). Subclasses provide
the concrete form widget and loader/saver calls.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Callable

import flet as ft

from ...core import loader, validation
from ...core.paths import list_json_files
from ..dialogs import confirm, snack
from ..widgets.file_list import FileList
from ..widgets.raw_editor import RawEditor


@dataclass
class ActionButton:
    label: str
    on_click: Callable
    kind: str = "filled"


class BaseResourceView(ft.Column):
    title: str = ""
    directory = None  # set in subclass
    name_field_label = "name"

    def __init__(self, page: ft.Page) -> None:
        super().__init__()
        self.page_ctx = page
        self.expand = True
        self.selected_name: str | None = None
        self.current_data: dict[str, Any] = {}
        self.file_list = FileList(
            names=[],
            on_select=self._on_select_name,
            on_new=self._on_new,
            title=self.title,
        )
        self.mode_toggle = ft.SegmentedButton(
            selected=["form"],
            segments=[
                ft.Segment(value="form", label=ft.Text("表单"), icon=ft.Icon(ft.Icons.EDIT_NOTE)),
                ft.Segment(value="raw", label=ft.Text("原始 JSON"), icon=ft.Icon(ft.Icons.DATA_OBJECT)),
            ],
            allow_multiple_selection=False,
            allow_empty_selection=False,
            on_change=self._on_mode_change,
        )
        self.form_container = ft.Container(expand=True, padding=10)
        self.raw_editor = RawEditor()
        self.raw_container = ft.Container(self.raw_editor, expand=True, visible=False)
        self.actions = ft.Row([], alignment=ft.MainAxisAlignment.END)
        self.controls = [
            ft.Row(
                [
                    ft.Container(self.file_list, width=280, expand=False),
                    ft.VerticalDivider(width=1),
                    ft.Column(
                        [self.mode_toggle, self.form_container, self.raw_container, self.actions],
                        expand=True,
                        spacing=10,
                    ),
                ],
                expand=True,
            ),
        ]
        self.refresh_list()

    # --- subclass hooks ---------------------------------------------------

    def load_all(self) -> list[dict[str, Any]]:
        """Return a list of raw dicts for the left list."""
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

    # --- layout -----------------------------------------------------------

    def _on_mode_change(self, e: ft.ControlEvent) -> None:
        mode = (self.mode_toggle.selected or ["form"])[0]
        if mode == "form":
            # Try to sync raw -> form view: parse raw and re-render form.
            import json as _json
            try:
                self.current_data = _json.loads(self.raw_editor.get_value())
            except _json.JSONDecodeError:
                pass
            self.form_container.visible = True
            self.raw_container.visible = False
        else:
            self.form_container.visible = False
            self.raw_container.visible = True
        self._render()

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
        self._render()

    def _on_new(self) -> None:
        self.current_data = self.blank_form_data()
        # Force a unique placeholder name so save does not collide.
        existing = set(self.names)
        base = "new"
        i = 1
        candidate = base
        while candidate in existing:
            candidate = f"{base}_{i}"
            i += 1
        self.current_data["name"] = candidate
        self.selected_name = None
        self._render()

    def _render(self) -> None:
        self.form_container.content = self.build_form(self.current_data)
        self.raw_editor.set_value(_pretty(self.current_data))
        self.actions.controls = self._action_buttons()
        self.form_container.update()
        self.raw_editor.update()
        self.actions.update()

    def _action_buttons(self) -> list[ft.Control]:
        buttons = [
            ft.FilledTonalButton("保存", on_click=self._on_save),
            ft.OutlinedButton(
                "删除",
                on_click=self._onDelete,
                disabled=self.selected_name is None,
            ),
            ft.OutlinedButton(
                "复制为新",
                on_click=self._onDuplicate,
                disabled=not self.current_data,
            ),
        ]
        return buttons

    def _current_payload(self) -> dict[str, Any] | None:
        if (self.mode_toggle.selected or ["form"])[0] == "raw":
            import json
            try:
                return json.loads(self.raw_editor.get_value())
            except json.JSONDecodeError as e:
                snack(self.page_ctx, f"JSON 解析失败: {e}", "error")
                return None
        return self.form_to_data()

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
            snack(self.page_ctx, f"保存失败: {ex}", "error")
            return
        self.selected_name = saved_name
        self.current_data = data
        self.refresh_list()
        snack(self.page_ctx, "已保存 " + saved_name, "ok")

    def _onDelete(self, _e: ft.ControlEvent) -> None:
        if self.selected_name is None:
            return

        def _do_delete():
            self.delete_data(self.selected_name)
            snack(self.page_ctx, f"已删除 {self.selected_name}", "ok")
            self.selected_name = None
            self.current_data = self.blank_form_data() if self.blank_form_data() else {}
            self.refresh_list()
            self._render()

        confirm(self.page_ctx, "确认删除", f"删除 {self.selected_name}?", _do_delete)

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
        self._render()

    def _on_tab_change(self, e: ft.ControlEvent) -> None:
        # Kept for compatibility; mode toggle handles switching now.
        pass


def _pretty(data: dict[str, Any]) -> str:
    import json
    return json.dumps(data, indent=4, ensure_ascii=False)