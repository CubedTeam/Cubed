import argparse
import copy
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
            logger.warning(
                f"Block: {block_path} is not Exists and Will Create A New One"
            )
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


def change_key(block: dict[str, Any], key: str, value: str):
    if type(block[key]) is str:
        block[key] = value
    elif type(block[key]) is int:
        try:
            v = int(value)
            block[key] = v
        except ValueError:
            logger.error("The Value Is Not A Int")
            return False
    elif type(block[key]) is bool:
        if value.lower() == "true" or value.lower() == "t":
            block[key] = True
        elif value.lower() == "false" or value.lower() == "f":
            block[key] = False
        else:
            logger.error("The Value Is Not A Bool")
            return False
    elif type(block[key]) is float:
        try:
            v = float(value)
            block[key] = v
        except ValueError:
            logger.error("The Value Is Not A Float")
            return False
    else:
        logger.error("Unkown Key Type")
        return False
    return True


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
        old_name = block[key]
        if change_key(block, key, value):
            print("Change Success")
            if key == "name":
                old_path: Path = data_path / (old_name + ".toml")
                try:
                    old_path.unlink()
                except FileNotFoundError:
                    logger.warning(
                        f"Name Change But Old File {old_name}.toml is Not  Exists!"
                    )
        else:
            print("Change Fail")
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
    data_exists = True
    if not data_path.exists():
        logger.error(f"Blocks Data Path {data_path} not Exists!")
        data_exists = False
    else:
        logger.info(f"Blocks Data Path {data_path}")
    texture_exists = True
    if not texture_path.exists():
        logger.error(f"Blocks Texture Path {texture_path} not Exists!")
        texture_exists = False
    else:
        logger.info(f"Blocks Texture Path {texture_path}")
    return data_exists and texture_exists


def check_integrity():
    find_error = False
    errors = 0
    if check_path():
        blocks = collect_blocks()
        template_path = data_path / "template.toml"
        if not template_path.is_file():
            logger.error("Template.toml is not Exists!")
            find_error = True
            errors += 1
            return
        template_block = pytomlpp.loads(template_path.read_text(encoding="utf-8"))
        n = len(blocks)
        for i in range(n):
            if "id" not in blocks[i]:
                logger.error(f"Id: {i} not Exists!")
                find_error = True
                errors += 1
                continue
            if blocks[i]["id"] != i:
                logger.error(
                    f"Id Error, Block {blocks[i].get('name', 'Unknow')} Id Should Be {i} Instead of {blocks[i]['id']}"
                )
                find_error = True
                errors += 1
            for key, value in template_block.items():
                if key not in blocks[i]:
                    logger.error(
                        f"Key Error, Block {blocks[i].get('name', 'Unknow')} Key {key} not Exists!"
                    )
                    find_error = True
                    errors += 1
                    continue
                if type(blocks[i][key]) is not type(value):
                    logger.error(
                        f"Value Type Error, Block {blocks[i].get(key, 'Unknow')} The Type Should Be {type(value)}, Instead of {type(blocks[i][key])}"
                    )
                    find_error = True
                    errors += 1
    if find_error:
        logger.error(f"Find {errors} Errors")
    else:
        logger.info("No Error")


def add_new_block():
    blocks = collect_blocks()
    template_path = data_path / "template.toml"
    if not template_path.is_file():
        logger.error("Template.toml is not Exists, Can't Create A New Block!")
        return
    template_block = pytomlpp.loads(template_path.read_text(encoding="utf-8"))
    new_block = copy.deepcopy(template_block)
    num = len(blocks)
    logger.info(f"New Block Id is {num}")
    new_block["id"] = num
    for key in template_block:
        if key == "id":
            continue
        nvalue = input(f"Input {key}: ")
        if not change_key(new_block, key, nvalue):
            logger.error(f"Add Key {key} Value {nvalue} Fail")
            return
    new_block_path: Path = data_path / (new_block["name"] + ".toml")
    new_block_path.write_text(pytomlpp.dumps(new_block))
    logger.info("Successfully Add New Block!")
    pprint(new_block)


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
    if args.check:
        check_integrity()
    if args.new:
        add_new_block()


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
    parser.add_argument(
        "-C", "--check", action="store_true", help="Check The Block Data Integrity"
    )
    parser.add_argument("-n", "--new", action="store_true", help="Add A New Block")


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
