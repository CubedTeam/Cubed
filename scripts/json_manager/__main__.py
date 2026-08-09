"""Entry point: ``uv run python -m scripts.json_manager``."""

from __future__ import annotations

import flet as ft

from .app import main


if __name__ == "__main__":
    ft.run(main)