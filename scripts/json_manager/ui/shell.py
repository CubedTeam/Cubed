"""Main shell: appbar + navigation rail + main area + status bar."""

from __future__ import annotations

import flet as ft

from ..core import i18n
from .status_bar import StatusBar
from .views.blocks_view import BlocksView
from .views.creatures_view import CreaturesView
from .views.items_view import ItemsView
from .views.lang_view import LangView
from .views.lexicon_view import LexiconView
from .views.registry_view import RegistryView
from .views.settings_view import SettingsView

SPACE = 12

NAV = [
    ("blocks", ft.Icons.BLOCK_OUTLINED, "nav.blocks", BlocksView),
    ("items", ft.Icons.INVENTORY_2_OUTLINED, "nav.items", ItemsView),
    ("creatures", ft.Icons.PETS_OUTLINED, "nav.creatures", CreaturesView),
    ("registry", ft.Icons.NUMBERS, "nav.registry", RegistryView),
    ("lang", ft.Icons.LANGUAGE, "nav.lang", LangView),
    ("lexicon", ft.Icons.WARNING_AMBER, "nav.lexicon", LexiconView),
    ("settings", ft.Icons.SETTINGS, "nav.settings", SettingsView),
]


class Shell(ft.Column):
    SHELL_KEY = "shell"

    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True, spacing=SPACE)
        self.page_ctx = page
        page.session.store.set(self.SHELL_KEY, self)

        self.title_text = ft.Text(
            i18n.t("app.title"), size=20, weight=ft.FontWeight.BOLD
        )
        self.theme_button = ft.IconButton(
            ft.Icons.LIGHT_MODE,
            on_click=self._toggle_theme,
            tooltip=i18n.t("action.theme_tip"),
        )
        self.refresh_button = ft.IconButton(
            ft.Icons.REFRESH,
            on_click=self._refresh_active,
            tooltip=i18n.t("action.refresh_tip"),
        )

        self.rail = ft.NavigationRail(
            selected_index=0,
            destinations=[
                ft.NavigationRailDestination(
                    icon=icon, label=i18n.t(label_key), selected_icon=icon
                )
                for _id, icon, label_key, _cls in NAV
            ],
            on_change=self._on_nav_change,
            extended=False,
            min_width=72,
            width=72,
            label_type=ft.NavigationRailLabelType.ALL,
        )

        self.main_area = ft.Container(expand=True, padding=SPACE, content=None)
        self.status_bar = StatusBar(page)

        self.controls = [
            # Top app bar.
            ft.Container(
                ft.Row(
                    [
                        self.title_text,
                        ft.Container(expand=True),
                        self.refresh_button,
                        self.theme_button,
                    ],
                    alignment=ft.MainAxisAlignment.SPACE_BETWEEN,
                    vertical_alignment=ft.CrossAxisAlignment.CENTER,
                ),
                padding=ft.Padding(left=8, right=SPACE, top=8, bottom=8),
            ),
            # Body: nav rail | divider | main area.
            ft.Row(
                [
                    self.rail,
                    ft.VerticalDivider(width=1),
                    self.main_area,
                ],
                expand=True,
                spacing=0,
            ),
            self.status_bar,
        ]
        self.view_instances: dict[str, object] = {}
        self._current_index = -1
        self._render_view(0)

    def _render_view(self, index: int) -> None:
        if index == self._current_index and self.main_area.content is not None:
            return
        self._current_index = index
        nav_id, _icon, _label, cls = NAV[index]
        if nav_id not in self.view_instances:
            self.view_instances[nav_id] = cls(self.page_ctx)
        view = self.view_instances[nav_id]
        if hasattr(view, "refresh"):
            try:
                view.refresh()  # type: ignore[attr-defined]
            except Exception:
                pass
        self.main_area.content = view
        try:
            self.main_area.update()
        except Exception:
            pass
        self.status_bar.refresh()

    def _on_nav_change(self, e: ft.ControlEvent) -> None:
        self._render_view(int(e.control.selected_index))

    def _toggle_theme(self, _e: ft.ControlEvent) -> None:
        if self.page_ctx.theme_mode == ft.ThemeMode.DARK:
            self.page_ctx.theme_mode = ft.ThemeMode.LIGHT
            self.theme_button.icon = ft.Icons.LIGHT_MODE
        else:
            self.page_ctx.theme_mode = ft.ThemeMode.DARK
            self.theme_button.icon = ft.Icons.DARK_MODE
        try:
            self.page_ctx.update()
        except Exception:
            pass

    def _refresh_active(self, _e: ft.ControlEvent) -> None:
        self._current_index = -1  # force re-render
        self._render_view(self.rail.selected_index or 0)
        self.status_bar.refresh()

    def apply_locale(self) -> None:
        """Rebuild all translatable chrome (title, rail, tooltips) and
        drop cached views so the next nav tick re-creates them in the
        new locale. Currently-edited data is not preserved; callers
        should warn the user before triggering this.
        """
        # Chrome.
        self.title_text.value = i18n.t("app.title")
        self.theme_button.tooltip = i18n.t("action.theme_tip")
        self.refresh_button.tooltip = i18n.t("action.refresh_tip")
        self.page_ctx.title = i18n.t("app.title")
        self.rail.destinations = [
            ft.NavigationRailDestination(
                icon=icon, label=i18n.t(label_key), selected_icon=icon
            )
            for _id, icon, label_key, _cls in NAV
        ]

        # Drop cached views so they get rebuilt with translated strings.
        self.view_instances.clear()
        self._current_index = -1

        try:
            self.title_text.update()
        except Exception:
            pass
        try:
            self.theme_button.update()
        except Exception:
            pass
        try:
            self.refresh_button.update()
        except Exception:
            pass
        try:
            self.rail.update()
        except Exception:
            pass

        self._render_view(self.rail.selected_index or 0)
        self.status_bar.refresh()
