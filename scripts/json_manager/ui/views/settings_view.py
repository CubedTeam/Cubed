"""Settings view: pick the UI locale and propagate it everywhere."""

from __future__ import annotations

import flet as ft

from ...core import i18n, settings
from .. import form
from ..dialogs import snack


class SettingsView(ft.Column):
    SHELL_KEY = "shell"

    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True, spacing=form.SPACE, scroll=ft.ScrollMode.AUTO)
        self.page_ctx = page

        self.lang_dropdown = ft.Dropdown(
            label=i18n.t("view.settings.section_language"),
            value=i18n.get_locale(),
            options=[ft.dropdown.Option(code) for code in i18n.available()],
            on_select=self._on_lang_change,
            expand=True,
        )
        self.help_text = ft.Text(
            i18n.t("view.settings.language_help"),
            size=12,
            color=ft.Colors.ON_SURFACE_VARIANT,
        )

        self.controls = [
            form.section(
                i18n.t("view.settings.section_language"),
                self.lang_dropdown,
                self.help_text,
            ),
        ]

    def _on_lang_change(self, e: ft.ControlEvent) -> None:
        new_locale = e.control.value
        if not new_locale or new_locale == i18n.get_locale():
            return
        if not i18n.set_locale(new_locale):
            snack(self.page_ctx, f"Unknown locale: {new_locale}", "error")
            return
        settings.save({**settings.load(), "locale": new_locale})
        shell = self.page_ctx.session.store.get(self.SHELL_KEY)
        if shell is not None and hasattr(shell, "apply_locale"):
            shell.apply_locale()
        snack(self.page_ctx, i18n.t("view.settings.language_updated"), "ok")
