"""Bottom status bar showing git status and the commit shortcut."""

from __future__ import annotations

import flet as ft

from ..core import git_ops
from .dialogs import prompt, snack


class StatusBar(ft.Container):
    def __init__(self, page: ft.Page) -> None:
        super().__init__()
        self.page_ctx = page
        self.text = ft.Text("git: 加载中…")
        self.btn_diff = ft.TextButton("查看 Diff", on_click=self._on_diff)
        self.btn_commit = ft.FilledTonalButton("Commit", on_click=self._on_commit)
        self.btn_refresh = ft.IconButton(ft.Icons.REFRESH, on_click=lambda _: self.refresh())
        self.content = ft.Row(
            [
                ft.Icon(ft.Icons.GRAPHIC_EQ),
                self.text,
                ft.Container(expand=True),
                self.btn_diff,
                self.btn_commit,
                self.btn_refresh,
            ],
            vertical_alignment=ft.CrossAxisAlignment.CENTER,
        )
        self.padding = ft.Padding(left=8, right=8, top=12, bottom=12)
        self.bgcolor = ft.Colors.SURFACE_CONTAINER_HIGHEST
        self.refresh()

    def refresh(self) -> None:
        if not git_ops.is_repo():
            self.text.value = "git: 未启用"
            self.btn_commit.disabled = True
            self.btn_diff.disabled = True
            self._update()
            return
        files = git_ops.status()
        modified = sum(
            1
            for f in files
            if f.tracked and f.status in ("M", "A", "D", "R", "C", "U")
        )
        untracked = sum(1 for f in files if not f.tracked)
        branch = git_ops.branch() or "(detached)"
        if not files:
            self.text.value = f"git [{branch}] 工作区干净"
        else:
            self.text.value = f"git [{branch}] {modified} modified · {untracked} untracked"
        self.btn_commit.disabled = not files
        self.btn_diff.disabled = not files
        self._update()

    def _update(self) -> None:
        try:
            self.text.update()
            self.btn_commit.update()
            self.btn_diff.update()
        except Exception:
            pass

    def _on_diff(self, _e: ft.ControlEvent) -> None:
        files = git_ops.status()
        from .widgets.diff_viewer import DiffViewer
        viewer = DiffViewer()
        joined = "\n".join(
            git_ops.diff(f.path) for f in files[:30]
        ) or "(no diff)"
        viewer.set_diff(joined)

        dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text("Git Diff"),
            content=ft.Container(viewer, width=900, height=600),
            actions=[ft.FilledButton("关闭", on_click=lambda e: e.page_ctx.close(dialog))],
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