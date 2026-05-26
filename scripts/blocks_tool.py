import argparse
import sys
from functools import singledispatch
from pathlib import Path
from pprint import pprint
from typing import Any

import pytomlpp
from loguru import logger

VERSION = "0.0.1"
DATA_PATH = "assets/data/block"
TEXTURE_PATH = "assets/texture/block"

work_path = Path(__file__).parent.parent
data_path = work_path / DATA_PATH
texture_path = work_path / TEXTURE_PATH


def collect_blocks() -> list[dict[str, Any]]:
    blocks: list[dict[str, Any]] = []
    for block in data_path.rglob("*.toml"):
        if not block.is_file():
            continue
        if block.name == "template.toml":
            continue

        blocks.append(pytomlpp.loads(block.read_text(encoding="utf-8")))
    blocks.sort(key=lambda x: x["id"])
    return blocks


def save_data(blocks: list[dict[str, Any]]):
    for block in blocks:
        block_path: Path = data_path / (block["name"] + ".toml")
        if not block_path.is_file():
            logger.error(f"Block: {block_path} is not Exists or a File")
            continue
        block_path.write_text(pytomlpp.dumps(block))


def sync_template_value():
    blocks = collect_blocks()
    template_path = data_path / "template.toml"
    if not template_path.is_file():
        logger.error("Template.toml is not Exists!")
        return
    template_block = pytomlpp.loads(template_path.read_text(encoding="utf-8"))
    add_count = 0
    for key, value in template_block.items():
        for block in blocks:
            if key not in block:
                block[key] = value
                add_count += 1
    save_data(blocks)
    logger.info(f"Synced {add_count} template fields to blocks")


@singledispatch
def show_data_info(arg: Any):
    logger.error("No Match show_data_info")


@show_data_info.register(type(None))
def _(arg: None):
    blocks = collect_blocks()
    print("Please Input Block Name or Id, Input exit or e to Exit")
    while True:
        input_str = input("Name or Id: ")
        try:
            id = int(input_str)
            if id >= len(blocks) or id < 0:
                print(f"Id: {id} Not Find, Input e or exit to Exit")
                continue
            pprint(blocks[id])
        except ValueError:
            if input_str.lower() == "exit" or input_str.lower() == "e":
                break
            find = False
            for block in blocks:
                if block["name"] == input_str:
                    pprint(block)
                    find = True
                    break
            if not find:
                print(f"Name: {input_str} Not Find, Input e or exit to Exit")


@show_data_info.register(int)
def _(id: int):
    blocks = collect_blocks()
    if id >= len(blocks) or id < 0:
        logger.error(f"ID: {id} is Not Invaild!")
        return
    pprint(blocks[id])


@show_data_info.register(str)
def _(name: str):
    blocks = collect_blocks()
    find = False
    for block in blocks:
        if block["name"] == name:
            pprint(block)
            find = True
            break
    if not find:
        logger.error(f"Block Name: {name} Not Find")


def handle_change(block: dict[str, Any]) -> dict[str, Any]:
    print("Please Input Block Key, Input exit or e to Exit")
    while True:
        key = input("Key: ")
        if key.lower() == "exit" or key.lower() == "e":
            break
        if key not in block:
            logger.error("The Key Is Not Exists!")
            continue
        value = input("Value: ")

        if isinstance(block[key], str):
            if value.lower() == "exit" or value.lower() == "e":
                print(f"The Value Is {value}, Do You Want to Exit or Write (Y/N)")
                ans = input()
                if ans.lower() == "y":
                    break
                elif ans.lower() != "n":
                    logger.error(f"Unknow {ans}")
                    continue
            block[key] = value
        elif isinstance(block[key], int):
            try:
                v = int(value)
                block[key] = v
            except ValueError:
                logger.error("The Value Is Not A Int")
                continue
        elif isinstance(block[key], bool):
            if value.lower() == "true":
                block[key] = True
            elif value.lower() == "false":
                block[key] = False
            else:
                logger.error("The Value Is Not A Bool")
                continue
        elif isinstance(block[key], float):
            try:
                v = float(value)
                block[key] = v
            except ValueError:
                logger.error("The Value Is Not A Float")
                continue
        else:
            logger.error("Unkown Key Type")
            continue
        print("Change Success")
        pprint(block)
    return block


@singledispatch
def change_data(arg: Any):
    logger.error("Not Match change")


@change_data.register(int)
def _(id: int):
    blocks = collect_blocks()
    if id >= len(blocks) or id < 0:
        logger.error(f"ID: {id} is Invaild!")
        return
    pprint(blocks[id])
    blocks[id] = handle_change(blocks[id])
    save_data(blocks)


@change_data.register(str)
def _(name: str):
    blocks = collect_blocks()
    find = False
    for i, block in enumerate(blocks):
        if block["name"] == name:
            pprint(block)
            blocks[i] = handle_change(block)
            save_data(blocks)
            find = True
            break
    if not find:
        logger.error(f"Block Name: {name} Not Find")


@change_data.register(type(None))
def _(arg: None):
    blocks = collect_blocks()
    print("Please Input Block Name or Id, Input exit or e to Exit")
    while True:
        input_str = input("Name or Id: ")
        try:
            id = int(input_str)
            if id >= len(blocks) or id < 0:
                print(f"Id: {id} Not Find, Input e or exit to Exit")
                continue
            pprint(blocks[id])
            blocks[id] = handle_change(blocks[id])
            save_data(blocks)
        except ValueError:
            if input_str.lower() == "exit" or input_str.lower() == "e":
                break
            find = False
            for i, block in enumerate(blocks):
                if block["name"] == input_str:
                    pprint(block)
                    blocks[i] = handle_change(block)
                    save_data(blocks)
                    find = True
                    break
            if not find:
                print(f"Name: {input_str} Not Find, Input e or exit to Exit")


def show_data_list():
    blocks = collect_blocks()
    for block in blocks:
        print(f"id: {block['id']} name: {block['name']}")


def check_path():

    logger.info(f"Work Path {work_path.resolve()}")
    logger.info(f"Script Dir {sys.path[0]}")

    if not data_path.exists():
        logger.error(f"Blocks Data Path {data_path} not Exists!")
    else:
        logger.info(f"Blocks Data Path {data_path}")

    if not texture_path.exists():
        logger.error(f"Blocks Texture Path {texture_path} not Exists!")
    else:
        logger.info(f"Blocks Texture Path {texture_path}")


def handle_args(args: argparse.Namespace):
    if args.version:
        print(f"Blocks Tools: {VERSION}")
        print(f"Python: {sys.version}")
    if args.path:
        check_path()
    if args.list:
        show_data_list()
    if args.sync:
        sync_template_value()
    if args.info:
        if args.info == "EMPTY":
            show_data_info(None)
        else:
            try:
                id = int(args.info)
                show_data_info(id)
            except ValueError:
                show_data_info(args.info)
    if args.change:
        if args.change == "EMPTY":
            change_data(None)
        else:
            try:
                id = int(args.change)
                change_data(id)
            except ValueError:
                change_data(args.change)


def init_parser(parser: argparse.ArgumentParser):
    parser.add_argument(
        "-v", "--version", action="store_true", help="Show Blocks Tools Version"
    )
    parser.add_argument(
        "--path", action="store_true", help="Check Blcoks Data and Texture Path"
    )
    parser.add_argument("-l", "--list", action="store_true", help="Show Blocks List")
    parser.add_argument(
        "-s",
        "--sync",
        action="store_true",
        help="Sync Template.toml Value to Other Toml, Only New Value Will Add",
    )
    parser.add_argument(
        "-i",
        "--info",
        nargs="?",
        const="EMPTY",
        help="Show Block Data, If Provide Id Will Print the Corresponding Blcok Data, You Can Input Id or Name",
    )
    parser.add_argument(
        "-c", "--change", nargs="?", const="EMPTY", help="Change Block Data"
    )


def main():
    parser = argparse.ArgumentParser(description="Block Manage Tool")

    init_parser(parser)

    if len(sys.argv) == 1:
        parser.print_help()
        exit(0)

    args = parser.parse_args()
    handle_args(args)


if __name__ == "__main__":
    main()
