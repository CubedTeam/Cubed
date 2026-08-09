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

NAV = [
    ("blocks", ft.Icons.BLOCK, "Blocks", BlocksView),
    ("items", ft.Icons.INVENTORY_2, "Items", ItemsView),
    ("creatures", ft.Icons.PETS, "Creatures", CreaturesView),
    ("registry", ft.Icons.NUMBERS, "Registry", RegistryView),
    ("lang", ft.Icons.LANGUAGE, "Lang", LangView),
    ("lexicon", ft.Icons.WARNING_AMBER, "Lexicon", LexiconView),
]


class Shell(ft.Column):
    def __init__(self, page: ft.Page) -> None:
        super().__init__(expand=True)
        self.page_ctx = page
        self.theme_button = ft.IconButton(ft.Icons.DARK_MODE, on_click=self._toggle_theme)
        self.refresh_button = ft.IconButton(ft.Icons.REFRESH, on_click=self._refresh_active)
        self.rail = ft.NavigationRail(
            selected_index=0,
            destinations=[
                ft.NavigationRailDestination(icon=icon, label=label)
                for _id, icon, label, _cls in NAV
            ],
            on_change=self._on_nav_change,
            extended=True,
            min_width=180,
        )
        self.main_area = ft.Column([ft.Text("select a category")], expand=True)
        self.status_bar = StatusBar(page)

        self.controls = [
            ft.Row(
                [
                    ft.Text(i18n.t("app.title"), size=22, weight=ft.FontWeight.BOLD),
                    ft.Container(expand=True),
                    self.refresh_button,
                    self.theme_button,
                ],
                alignment=ft.MainAxisAlignment.SPACE_BETWEEN,
            ),
            ft.Row(
                [
                    self.rail,
                    ft.VerticalDivider(width=1),
                    ft.Container(self.main_area, expand=True, padding=10),
                ],
                expand=True,
            ),
            self.status_bar,
        ]
        self.view_instances: dict[str, object] = {}
        self._render_view(0)

    def _render_view(self, index: int) -> None:
        nav_id, _icon, _label, cls = NAV[index]
        if nav_id not in self.view_instances:
            self.view_instances[nav_id] = cls(self.page_ctx)
        view = self.view_instances[nav_id]
        # Views expose refresh() for resource views; lang/lexicon also have refresh().
        if hasattr(view, "refresh"):
            try:
                view.refresh()  # type: ignore[attr-defined]
            except Exception:
                pass
        self.main_area.controls = [view]
        self.main_area.update()

    def _on_nav_change(self, e: ft.ControlEvent) -> None:
        self._render_view(int(e.control.selected_index))

    def _toggle_theme(self, e: ft.ControlEvent) -> None:
        if self.page_ctx.theme_mode == ft.ThemeMode.DARK:
            self.page_ctx.theme_mode = ft.ThemeMode.LIGHT
            self.theme_button.icon = ft.Icons.DARK_MODE
        else:
            self.page_ctx.theme_mode = ft.ThemeMode.DARK
            self.theme_button.icon = ft.Icons.LIGHT_MODE
        self.page_ctx.update()

    def _refresh_active(self, _e: ft.ControlEvent) -> None:
        idx = self.rail.selected_index or 0
        self._render_view(idx)
        self.status_bar.refresh()