"""Import/audit PDO documentation and sensor definitions without executing upstream code."""

import argparse
import ast
import json
import re
from pathlib import Path
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]
TARGET = ROOT / "components" / "zehnder_comfoair_q" / "pdo_mapping.json"
DEFAULT_REVISION = "8fe395b3f8657e25ef8c5a6ccea5aab06808a514"


def fetch(revision, path):
    with urlopen(f"https://raw.githubusercontent.com/michaelarnauts/aiocomfoconnect/{revision}/{path}", timeout=30) as response:
        return response.read().decode("utf-8")


def import_reference(revision):
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise ValueError("Use a full pinned commit SHA")
    document = fetch(revision, "docs/PROTOCOL-PDO.md").split("# Overview of known sensors", 1)[1]
    entries = {}
    for line in document.splitlines():
        if not re.match(r"\|\s*\d+\s*\|", line):
            continue
        cells = [cell.strip() for cell in line.split("|")[1:-1]]
        pdo = int(cells[0])
        if pdo in entries:
            raise ValueError(f"Duplicate documented PDO {pdo}")
        entries[pdo] = {
            "id": pdo, "description": cells[2], "documented_type": cells[1] or None,
            "examples": cells[3], "registry_type": None, "unit": None,
            "registry_value_fn": None, "type": cells[1] or None,
        }
    source = ast.parse(fetch(revision, "aiocomfoconnect/sensors.py"))
    units = {}
    for node in source.body:
        if isinstance(node, ast.Assign) and isinstance(node.value, ast.Constant):
            units.update({target.id: node.value.value for target in node.targets if isinstance(target, ast.Name)})
    for node in source.body:
        if not isinstance(node, ast.AnnAssign) or getattr(node.target, "id", None) != "SENSORS":
            continue
        for sensor in node.value.values:
            pdo = ast.literal_eval(sensor.args[2])
            entry = entries[pdo]
            entry["registry_type"] = sensor.args[3].attr.removeprefix("TYPE_")
            entry["registry_name"] = ast.literal_eval(sensor.args[0])
            unit = sensor.args[1]
            entry["unit"] = units[unit.id] if isinstance(unit, ast.Name) else ast.literal_eval(unit)
            if len(sensor.args) > 4:
                entry["registry_value_fn"] = ast.unparse(sensor.args[4])
            entry["type"] = entry["registry_type"]
    entries[212]["note"] = "Documentation says UINT8; registry declares INT16. KD returned two bytes E9 00 (233). Width is observed; signedness follows the registry. No alternate type is accepted."
    for pdo in (213, 216):
        entries[pdo]["note"] = "Documentation examples imply /100; Python treats raw values as watts. Scaling remains unverified."
    return {"upstream_repository": "michaelarnauts/aiocomfoconnect", "upstream_revision": revision,
            "pdos": [entries[pdo] for pdo in sorted(entries)]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--revision", default=DEFAULT_REVISION)
    parser.add_argument("--check", action="store_true", help="Compare the pinned upstream data without writing")
    args = parser.parse_args()
    mapping = import_reference(args.revision)
    if args.check:
        if mapping != json.loads(TARGET.read_text(encoding="utf-8")):
            parser.error("Upstream reference differs; review and synchronize pdo_mapping.json")
    else:
        TARGET.write_text(json.dumps(mapping, indent=2, ensure_ascii=True) + "\n", encoding="utf-8", newline="\n")
    print(f"Checked {len(mapping['pdos'])} PDO definitions at {args.revision}")


if __name__ == "__main__":
    main()
