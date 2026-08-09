"""Bottom status bar showing git status and the commit shortcut."""

from __future__ import annotations

import flet as ft

from ..core import git_ops, i18n
from .dialogs import prompt, snack
from .safe import safe_update
from .widgets.diff_viewer import DiffViewer

PADDING = ft.Padding(left=12, right=12, top=8, bottom=8)


class StatusBar(ft.Container):
    def __init__(self, page: ft.Page) -> None:
        super().__init__()
        self.page_ctx = page
        self.text = ft.Text(i18n.t("status.git_loading"), size=13)
        self.branch_chip = ft.Container(
            ft.Text("", size=12),
            padding=ft.Padding(left=8, right=8, top=4, bottom=4),
            border_radius=8,
            bgcolor=ft.Colors.SECONDARY_CONTAINER,
            visible=False,
        )
        self.btn_diff = ft.TextButton(
            i18n.t("status.view_diff"),
            icon=ft.Icons.DIFFERENCE,
            on_click=self._on_diff,
            disabled=True,
        )
        self.btn_commit = ft.FilledTonalButton(
            i18n.t("status.commit"),
            icon=ft.Icons.COMMIT,
            on_click=self._on_commit,
            disabled=True,
        )
        self.btn_refresh = ft.IconButton(
            ft.Icons.REFRESH,
            on_click=lambda _: self.refresh(),
            tooltip=i18n.t("status.refresh_tip"),
        )
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
            self.text.value = i18n.t("status.git_unavailable")
            self.btn_commit.disabled = True
            self.btn_diff.disabled = True
            self.branch_chip.visible = False
            self._update()
            return
        files = git_ops.status()
        modified = sum(
            1 for f in files if f.tracked and f.status in ("M", "A", "D", "R", "C", "U")
        )
        untracked = sum(1 for f in files if not f.tracked)
        branch = git_ops.branch() or i18n.t("status.git_detached")
        self.branch_chip.content.value = branch  # type: ignore[union-attr]
        self.branch_chip.visible = True
        if not files:
            self.text.value = i18n.t("status.git_clean")
            self.text.color = ft.Colors.ON_SURFACE_VARIANT
        else:
            self.text.value = i18n.t(
                "status.modified_untracked", modified=modified, untracked=untracked
            )
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
        joined = "\n".join(git_ops.diff(f.path) for f in files[:30]) or i18n.t(
            "status.no_diff"
        )
        viewer.set_diff(joined)
        dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text(i18n.t("status.git_diff_title")),
            content=ft.Container(viewer, width=900, height=600),
            actions=[
                ft.FilledButton(
                    i18n.t("status.close"), on_click=lambda e: _close(e, dialog)
                )
            ],
        )
        self.page_ctx.show_dialog(dialog)

    def _on_commit(self, _e: ft.ControlEvent) -> None:
        files = git_ops.status()

        def _commit(message: str) -> None:
            paths = [f.path for f in files]
            ok = git_ops.commit(message, paths)
            if ok:
                snack(self.page_ctx, i18n.t("status.committed"), "ok")
            else:
                snack(self.page_ctx, i18n.t("status.commit_failed"), "error")
            self.refresh()

        prompt(
            self.page_ctx,
            i18n.t("status.commit_title"),
            _commit,
            label=i18n.t("status.commit_message_label"),
            initial=i18n.t("status.commit_initial"),
        )


def _close(e: ft.ControlEvent, dialog: ft.AlertDialog) -> None:
    try:
        e.page.pop_dialog()
    except Exception:
        pass
