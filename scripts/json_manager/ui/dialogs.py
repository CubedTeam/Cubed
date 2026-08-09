"""Reusable MD3 dialogs/toasts.

Uses Flet 0.86's ``page.show_dialog()`` / ``page.pop_dialog()`` API so the
caller does not juggle overlay bookkeeping.
"""

from __future__ import annotations

from typing import Callable

import flet as ft

from ..core import i18n


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
            ft.TextButton(
                i18n.t("action.cancel"), on_click=lambda e: close(e, on_cancel)
            ),
            ft.FilledButton(
                i18n.t("action.confirm"), on_click=lambda e: close(e, on_confirm)
            ),
        ],
    )
    page.show_dialog(dialog)


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
            ft.TextButton(
                i18n.t("action.cancel"), on_click=lambda e: close(e, on_cancel)
            ),
            ft.FilledButton(i18n.t("action.ok"), on_click=submit),
        ],
        actions_alignment=ft.MainAxisAlignment.END,
    )
    # Enter key submits.
    field.on_submit = submit
    page.show_dialog(dialog)


def close(e, callback: Callable[[], None] | None) -> None:
    """Close the topmost open dialog and run callback."""
    try:
        e.page.pop_dialog()
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
    page.show_dialog(
        ft.SnackBar(
            content=ft.Text(message, color=ft.Colors.WHITE),
            bgcolor=color,
        )
    )
