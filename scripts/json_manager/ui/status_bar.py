"""Bottom status bar showing git status and the commit shortcut."""

from __future__ import annotations

import flet as ft

from ..core import git_ops
from .dialogs import prompt, snack
from .safe import safe_update
from .widgets.diff_viewer import DiffViewer

PADDING = ft.Padding(left=12, right=12, top=8, bottom=8)


class StatusBar(ft.Container):
    def __init__(self, page: ft.Page) -> None:
        super().__init__()
        self.page_ctx = page
        self.text = ft.Text("git: 加载中…", size=13)
        self.branch_chip = ft.Container(
            ft.Text("", size=12),
            padding=ft.Padding(left=8, right=8, top=4, bottom=4),
            border_radius=8,
            bgcolor=ft.Colors.SECONDARY_CONTAINER,
            visible=False,
        )
        self.btn_diff = ft.TextButton("查看 Diff", icon=ft.Icons.DIFFERENCE, on_click=self._on_diff, disabled=True)
        self.btn_commit = ft.FilledTonalButton("Commit", icon=ft.Icons.COMMIT, on_click=self._on_commit, disabled=True)
        self.btn_refresh = ft.IconButton(ft.Icons.REFRESH, on_click=lambda _: self.refresh(), tooltip="刷新")
        self.content = ft.Row(
            [
                ft.Icon(ft.Icons.GRAPHIC_EQ, size=18),
                self.branch_chip,
                self.text,
                ft.Container(expand=True),
                self.btn_diff,
                self.btn_commit,
                self.btn_refresh,
            ],
            vertical_alignment=ft.CrossAxisAlignment.CENTER,
            spacing=8,
        )
        self.padding = PADDING
        self.bgcolor = ft.Colors.SURFACE_CONTAINER_HIGHEST
        self.refresh()

    def refresh(self) -> None:
        if not git_ops.is_repo():
            self.text.value = "git: 未启用"
            self.btn_commit.disabled = True
            self.btn_diff.disabled = True
            self.branch_chip.visible = False
            self._update()
            return
        files = git_ops.status()
        modified = sum(1 for f in files if f.tracked and f.status in ("M", "A", "D", "R", "C", "U"))
        untracked = sum(1 for f in files if not f.tracked)
        branch = git_ops.branch() or "(detached)"
        self.branch_chip.content.value = branch  # type: ignore[union-attr]
        self.branch_chip.visible = True
        if not files:
            self.text.value = "工作区干净"
            self.text.color = ft.Colors.ON_SURFACE_VARIANT
        else:
            self.text.value = f"{modified} modified · {untracked} untracked"
            self.text.color = ft.Colors.ON_SURFACE
        self.btn_commit.disabled = not files
        self.btn_diff.disabled = not files
        self._update()

    def _update(self) -> None:
        safe_update(self.text)
        safe_update(self.branch_chip)
        safe_update(self.btn_commit)
        safe_update(self.btn_diff)

    def _on_diff(self, _e: ft.ControlEvent) -> None:
        files = git_ops.status()
        viewer = DiffViewer()
        joined = "\n".join(git_ops.diff(f.path) for f in files[:30]) or "(no diff)"
        viewer.set_diff(joined)
        dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text("Git Diff"),
            content=ft.Container(viewer, width=900, height=600),
            actions=[ft.FilledButton("关闭", on_click=lambda e: _close(e, dialog))],
        )
        self.page_ctx.open(dialog)

    def _on_commit(self, _e: ft.ControlEvent) -> None:
        files = git_ops.status()

        def _commit(message: str) -> None:
            paths = [f.path for f in files]
            ok = git_ops.commit(message, paths)
            if ok:
                snack(self.page_ctx, "已提交", "ok")
            else:
                snack(self.page_ctx, "提交失败 (检查 git 输出)", "error")
            self.refresh()

        prompt(
            self.page_ctx,
            "Commit",
            _commit,
            label="commit message",
            initial="chore(resources): update json",
        )


def _close(e: ft.ControlEvent, dialog: ft.AlertDialog) -> None:
    try:
        e.page.close(dialog)
    except Exception:
        pass