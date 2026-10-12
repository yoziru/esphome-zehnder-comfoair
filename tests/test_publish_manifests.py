"""Publishing must keep firmware for boards with the same chip separate."""

import json
from pathlib import Path
import re
import tempfile
import unittest

from scripts.build_manifests import build_manifests


REPO = Path(__file__).resolve().parents[1]
BOARDS = {
    "m5stack-atom": "ESP32",
    "m5stack-atoms3": "ESP32-S3",
    "m5stack-nanoc6": "ESP32-C6",
    "waveshare-esp32-s3-rs485-can": "ESP32-S3",
}
FILES = [f"zehnder-comfoair-q-{board}.yml" for board in BOARDS]


class PublishManifestsTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.output = Path(self.temp.name)
        for file, family in zip(FILES, BOARDS.values()):
            directory = self.output / Path(file).stem
            directory.mkdir()
            # The build action uses chip-based filenames, shared by both S3 boards.
            prefix = "zehnder-comfoair-q-" + family.lower().replace("-", "")
            build = {"chipFamily": family, "ota": {"path": prefix + ".ota.bin", "md5": "test"},
                     "parts": [{"path": prefix + ".factory.bin", "offset": 0}]}
            for part in [build["ota"], *build["parts"]]:
                (directory / part["path"]).write_bytes(file.encode())
            (directory / "manifest.json").write_text(json.dumps(build))
            (directory / "project_version").write_text("2026.9.1\n")

    def generate(self, legacy_files=None):
        build_manifests(self.output, FILES, legacy_files or FILES[:3],
                        "Zehnder ComfoAir Q", "manifest.json")

    def test_each_board_selects_its_own_firmware(self):
        self.generate()
        for file, family in zip(FILES, BOARDS.values()):
            directory = self.output / Path(file).stem
            manifest = json.loads((directory / "install.json").read_text())
            self.assertEqual(manifest["name"], "Zehnder ComfoAir Q")
            self.assertEqual(manifest["version"], "2026.9.1")
            self.assertEqual(manifest["home_assistant_domain"], "esphome")
            self.assertEqual(len(manifest["builds"]), 1)
            build = manifest["builds"][0]
            self.assertEqual(build["chipFamily"], family)
            self.assertEqual(build["parts"][0]["offset"], 0)
            self.assertEqual(build["ota"]["md5"], "test")
            for part in [build["ota"], *build["parts"]]:
                self.assertEqual((directory / part["path"]).read_bytes(), file.encode())

    def test_legacy_manifest_preserves_previous_boards(self):
        self.generate()
        manifest = json.loads((self.output / "manifest.json").read_text())
        self.assertEqual([build["chipFamily"] for build in manifest["builds"]],
                         ["ESP32", "ESP32-S3", "ESP32-C6"])
        for build, file in zip(manifest["builds"], FILES[:3]):
            for part in [build["ota"], *build["parts"]]:
                self.assertEqual((self.output / part["path"]).read_bytes(), file.encode())

    def test_installer_buttons_match_published_boards(self):
        self.generate()
        page = (REPO / "static/index.md").read_text()
        manifests = re.findall(r'<esp-web-install-button manifest="\./([^"]+)"', page)
        self.assertEqual(set(manifests), {f"{Path(file).stem}/install.json" for file in FILES})
        self.assertEqual(len(manifests), len(FILES))
        for manifest in manifests:
            self.assertTrue((self.output / manifest).is_file())
        workflow = (REPO / ".github/workflows/publish.yml").read_text()
        self.assertEqual(re.search(r"^      files: (.+)$", workflow, re.M)[1].split(","), FILES)
        self.assertEqual(re.search(r"^      legacy_files: (.+)$", workflow, re.M)[1].split(","), FILES[:3])

    def test_missing_artifacts_prevent_publication(self):
        directory = self.output / Path(FILES[-1]).stem
        for path in list(directory.iterdir()):
            with self.subTest(missing=path.name):
                contents = path.read_bytes()
                path.unlink()
                with self.assertRaises((OSError, ValueError)):
                    self.generate()
                self.assertFalse((self.output / "manifest.json").exists())
                path.write_bytes(contents)

    def test_duplicate_chip_family_in_legacy_manifest_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "same chip family"):
            self.generate(legacy_files=FILES)

    def test_inconsistent_versions_are_rejected(self):
        (self.output / Path(FILES[-1]).stem / "project_version").write_text("old-version")
        with self.assertRaisesRegex(ValueError, "same nonempty project version"):
            self.generate()

    def test_missing_flash_parts_are_rejected(self):
        path = self.output / Path(FILES[-1]).stem / "manifest.json"
        build = json.loads(path.read_text())
        build["parts"] = []
        path.write_text(json.dumps(build))
        with self.assertRaisesRegex(ValueError, "flash parts"):
            self.generate()


if __name__ == "__main__":
    unittest.main()
