"""Reusable MD3 dialogs/toasts.

Uses Flet 0.86's ``page.open()`` / ``page.close()`` overlay API so the
caller does not juggle overlay bookkeeping.
"""

from __future__ import annotations

from typing import Callable

import flet as ft


def confirm(
    page: ft.Page,
    title: str,
    body: str,
    on_confirm: Callable[[], None],
    on_cancel: Callable[[], None] | None = None,
) -> None:
    dialog = ft.AlertDialog(
        modal=True,
        title=ft.Text(title),
        content=ft.Text(body),
        actions=[
            ft.TextButton("取消", on_click=lambda e: close(e, on_cancel)),
            ft.FilledButton("确认", on_click=lambda e: close(e, on_confirm)),
        ],
    )
    page.open(dialog)


def prompt(
    page: ft.Page,
    title: str,
    on_submit: Callable[[str], None],
    initial: str = "",
    label: str | None = None,
    on_cancel: Callable[[], None] | None = None,
) -> None:
    field = ft.TextField(
        label=label or title,
        value=initial,
        autofocus=True,
    )

    def submit(e):
        close(e, lambda: on_submit(field.value or ""))

    dialog = ft.AlertDialog(
        modal=True,
        title=ft.Text(title),
        content=field,
        actions=[
            ft.TextButton("取消", on_click=lambda e: close(e, on_cancel)),
            ft.FilledButton("确定", on_click=submit),
        ],
        actions_alignment=ft.MainAxisAlignment.END,
    )
    # Enter key submits.
    field.on_submit = submit
    page.open(dialog)


def close(e, callback: Callable[[], None] | None) -> None:
    """Close the nearest open dialog belonging to e and run callback."""
    try:
        # Find the AlertDialog ancestor in the overlay tree.
        node = e.control
        while node is not None and not isinstance(node, ft.AlertDialog):
            node = getattr(node, "parent", None)
        if isinstance(node, ft.AlertDialog):
            e.page.close(node)
        else:
            e.page.close()
    except Exception:
        pass
    if callback:
        callback()


def snack(page: ft.Page, message: str, kind: str = "info") -> None:
    color = {
        "info": ft.Colors.ON_SURFACE_VARIANT,
        "ok": ft.Colors.GREEN_700,
        "error": ft.Colors.RED_700,
    }.get(kind, ft.Colors.ON_SURFACE_VARIANT)
    page.open(
        ft.SnackBar(
            content=ft.Text(message, color=ft.Colors.WHITE),
            bgcolor=color,
        )
    )