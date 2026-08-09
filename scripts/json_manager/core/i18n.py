"""Small i18n stub for the tool UI itself.

Falls back to English when a key is missing so the tool always renders
even if a translator has not finished a locale yet.
"""

from __future__ import annotations

_STRINGS: dict[str, dict[str, str]] = {
    "zh_CN": {
        "app.title": "Cubed 资源管理",
        "nav.blocks": "方块",
        "nav.items": "物品",
        "nav.creatures": "生物",
        "nav.registry": "注册表",
        "nav.lang": "语言",
        "nav.lexicon": "敏感词",
        "action.new": "新建",
        "action.save": "保存",
        "action.delete": "删除",
        "action.duplicate": "复制",
        "action.refresh": "刷新",
        "action.commit": "提交",
        "action.view_diff": "查看 Diff",
        "action.unlock": "解锁编辑",
        "status.modified": "已修改",
        "status.untracked": "未跟踪",
        "status.clean": "工作区干净",
        "msg.saved": "已保存",
        "msg.deleted": "已删除",
        "msg.invalid": "校验失败",
        "msg.confirm_delete": "确认删除？",
        "view.form": "表单",
        "view.raw": "原始 JSON",
    },
    "en_US": {
        "app.title": "Cubed Resources",
        "nav.blocks": "Blocks",
        "nav.items": "Items",
        "nav.creatures": "Creatures",
        "nav.registry": "Registry",
        "nav.lang": "Lang",
        "nav.lexicon": "Lexicon",
        "action.new": "New",
        "action.save": "Save",
        "action.delete": "Delete",
        "action.duplicate": "Duplicate",
        "action.refresh": "Refresh",
        "action.commit": "Commit",
        "action.view_diff": "View Diff",
        "action.unlock": "Unlock",
        "status.modified": "modified",
        "status.untracked": "untracked",
        "status.clean": "clean",
        "msg.saved": "Saved",
        "msg.deleted": "Deleted",
        "msg.invalid": "Validation failed",
        "msg.confirm_delete": "Confirm delete?",
        "view.form": "Form",
        "view.raw": "Raw JSON",
    },
}

_current = "zh_CN"


def set_locale(locale: str) -> None:
    global _current
    if locale in _STRINGS:
        _current = locale


def t(key: str, default: str | None = None) -> str:
    return _STRINGS.get(_current, {}).get(key, default or key)