"""Offline mapping/generation contracts. Run with python -m unittest discover -s tests."""

import importlib.util
import json
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
COMPONENT = ROOT / "components" / "zehnder_comfoair_q"
spec = importlib.util.spec_from_file_location("generate_pdo_map", ROOT / "scripts/generate_pdo_map.py")
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


class MappingTests(unittest.TestCase):
    def setUp(self):
        self.mapping = json.loads((COMPONENT / "pdo_mapping.json").read_text(encoding="utf-8"))
        self.entries = {entry["id"]: entry for entry in self.mapping["pdos"]}

    def test_pinned_inventory(self):
        self.assertRegex(self.mapping["upstream_revision"], r"^[0-9a-f]{40}$")
        self.assertEqual(len(self.entries), 158)
        self.assertEqual(sum(entry["registry_type"] is not None for entry in self.entries.values()), 86)

    def test_generated_table_is_current(self):
        self.assertEqual(generator.render(self.mapping), (COMPONENT / "pdo_types.h").read_text(encoding="utf-8"))

    def test_one_explicit_type_per_pdo(self):
        for entry in self.entries.values():
            self.assertNotIn("accepted_types", entry)
            self.assertTrue(entry["type"] is None or entry["type"] in generator.TYPES)
        self.assertEqual(self.entries[212]["type"], "CN_INT16")
        self.assertEqual(self.entries[212]["documented_type"], "CN_UINT8")
        self.assertIsNone(self.entries[69]["type"])
        self.assertNotIn(305, self.entries)
        self.assertNotIn(306, self.entries)

    def test_existing_dispatch_covered(self):
        text = (ROOT / "packages/canbus.yml").read_text(encoding="utf-8")
        matches = re.findall(r"comfoair_on_frame_case\(\s*(\d+)|case\s+(\d+)\s*:", text)
        ids = {int(macro_id or case_id) for macro_id, case_id in matches}
        for pdo in ids | {226, 230}:
            self.assertTrue(self.entries[pdo]["type"], f"PDO {pdo} has a known type")

    def test_invalid_mapping_is_rejected(self):
        for entries in ([self.entries[16], self.entries[16]], [self.entries[49], self.entries[16]]):
            mapping = {**self.mapping, "pdos": entries}
            with self.assertRaises(ValueError):
                generator.render(mapping)
        entry = {**self.entries[16], "type": "CN_STRING"}
        with self.assertRaises(ValueError):
            generator.render({**self.mapping, "pdos": [entry]})


if __name__ == "__main__":
    unittest.main()
