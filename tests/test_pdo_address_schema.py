"""Check configured high-ID bounds and the actual YAML filter against namespaces."""

import importlib.util
from pathlib import Path
import unittest

from esphome import config_validation as cv
import yaml

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("address_component", ROOT / "components/zehnder_comfoair_q/__init__.py")
component = importlib.util.module_from_spec(spec)
spec.loader.exec_module(component)


class AddressTests(unittest.TestCase):
    def test_high_ids_and_rejection(self):
        config = component.CONFIG_SCHEMA({"id": "comfoair", "canbus_id": "bus", "request_ids": [1024, 1304, 2047]})
        self.assertEqual(config["request_ids"], [1024, 1304, 2047])
        for pdo in (-1, 2048, 65535):
            with self.assertRaises(cv.Invalid):
                component.CONFIG_SCHEMA({"id": "comfoair", "canbus_id": "bus", "request_ids": [pdo]})

    def test_production_filter(self):
        package = yaml.safe_load((ROOT / "packages/canbus.yml").read_text())
        frame = package["canbus"][0]["on_frame"][1]
        self.assertTrue(frame["use_extended_id"])
        self.assertFalse(frame["remote_transmission_request"])
        for pdo in (16, 1023, 1024, 1304, 2047):
            self.assertEqual(((pdo << 14) | 0x41) & frame["can_id_mask"], frame["can_id"])
        for unrelated in (0x10000001, 0x1f000a81, 0x1f800000, 0x02000041):
            self.assertNotEqual(unrelated & frame["can_id_mask"], frame["can_id"])


if __name__ == "__main__":
    unittest.main()
