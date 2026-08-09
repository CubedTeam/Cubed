"""Diff viewer (colors added/removed lines)."""

from __future__ import annotations

import flet as ft


class DiffViewer(ft.Column):
    def __init__(self) -> None:
        super().__init__()
        self.expand = True
        self.list_view = ft.ListView(expand=True, spacing=0, auto_scroll=True)

    def set_diff(self, diff_text: str) -> None:
        lines: list[ft.Control] = []
        for line in (diff_text or "").splitlines():
            if line.startswith("+"):
                col = ft.Colors.GREEN_300 if line.startswith("+++") else ft.Colors.GREEN_700
                lines.append(
                    ft.Container(
                        ft.Text(line, font_family="monospace", selectable=True, color=col),
                        bgcolor=ft.Colors.with_opacity(0.08, ft.Colors.GREEN),
                    )
                )
            elif line.startswith("-"):
                col = ft.Colors.RED_300 if line.startswith("---") else ft.Colors.RED_700
                lines.append(
                    ft.Container(
                        ft.Text(line, font_family="monospace", selectable=True, color=col),
                        bgcolor=ft.Colors.with_opacity(0.08, ft.Colors.RED),
                    )
                )
            else:
                lines.append(
                    ft.Text(line or "\u00a0", font_family="monospace", selectable=True)
                )
        self.list_view.controls = lines or [ft.Text("(no diff)")]
        self.list_view.update()


def diff_dialog(diff_text: str, path_label: str) -> ft.AlertDialog:
    viewer = DiffViewer()
    viewer.set_diff(diff_text)
    return ft.AlertDialog(
        modal=True,
        title=ft.Text(f"Diff: {path_label}"),
        content=ft.Container(viewer, width=900, height=600),
        actions=[ft.FilledButton("关闭", on_click=lambda e: e.page.close_all())],
    )