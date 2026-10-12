#!/usr/bin/env python3
"""Wrap ESPHome build manifests for board-specific web installation."""

import argparse
import copy
import json
from pathlib import Path


def build_manifests(output, files, legacy_files, name, manifest_filename):
    boards = [Path(file).stem for file in files]
    legacy_boards = [Path(file).stem for file in legacy_files]
    if not boards or not legacy_boards or not set(legacy_boards) <= set(boards):
        raise ValueError("Legacy configurations must be included in the published files")

    builds = {}
    versions = set()
    for board in boards:
        directory = output / board
        build = json.loads((directory / "manifest.json").read_text())
        versions.add((directory / "project_version").read_text().strip())
        if not build.get("chipFamily") or not build.get("parts"):
            raise ValueError(f"Missing chip family or flash parts for {board}")
        for part in [build["ota"], *build["parts"]]:
            if not (directory / part["path"]).is_file():
                raise ValueError(f"Manifest references missing file: {board}/{part['path']}")
        builds[board] = build

    if len(versions) != 1 or not next(iter(versions)):
        raise ValueError("All boards must have the same nonempty project version")
    metadata = {
        "name": name,
        "version": versions.pop(),
        "home_assistant_domain": "esphome",
        "new_install_skip_erase": False,
    }

    # Keep the existing combined manifest unambiguous for older links.
    legacy_builds = []
    families = set()
    for board in legacy_boards:
        build = copy.deepcopy(builds[board])
        if build["chipFamily"] in families:
            raise ValueError("Legacy manifest cannot contain two boards of the same chip family")
        families.add(build["chipFamily"])
        for part in [build["ota"], *build["parts"]]:
            part["path"] = f"{board}/{part['path']}"
        legacy_builds.append(build)

    for board, build in builds.items():
        (output / board / "install.json").write_text(
            json.dumps({**metadata, "builds": [build]}, indent=2) + "\n"
        )
    (output / manifest_filename).write_text(
        json.dumps({**metadata, "builds": legacy_builds}, indent=2) + "\n"
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--files", required=True, help="Comma-separated configuration files")
    parser.add_argument("--legacy-files", required=True, help="Configurations for the combined manifest")
    parser.add_argument("--name", required=True)
    parser.add_argument("--manifest-filename", default="manifest.json")
    args = parser.parse_args()
    try:
        build_manifests(
            args.output, args.files.split(","), args.legacy_files.split(","),
            args.name, args.manifest_filename,
        )
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, f"Manifest generation failed: {error}\n")


if __name__ == "__main__":
    main()
