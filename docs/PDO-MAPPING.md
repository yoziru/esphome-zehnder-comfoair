# PDO Type Mapping

`components/zehnder_comfoair_q/pdo_mapping.json` records every documented PDO
at a pinned aiocomfoconnect revision, the Python sensor registry's type/unit/
conversion where present, and exactly one declared runtime type. This inventory
does not enable new sensors, change CAN addressing, or poll unknown accessories.

`scripts/generate_pdo_map.py` generates `pdo_types.h` from that data. The runtime
table deliberately contains only IDs and their declared types; labels, units, and
scaling remain in existing YAML entities until a separately reviewed migration.
`pdo_decode.h` handles BOOL, UINT8/16/32, and INT8/16/64 in little-endian form.
All integer and bitset decoding stays in integer form until sensor publication.

`comfoair_on_frame_case(pdo_id, component, entity)` takes three arguments. Each
known PDO uses the table's single type and exact width. Unknown PDOs and invalid
payloads are rejected without publishing a replacement state. There are no
signedness flags, alternate types, length-inference decoders, or old-API aliases.
Call sites must use the current interface, not a compatibility wrapper.

Known handlers can call `publish_pdo_state(entity, bytes, pdo_id)` directly.
Experimental raw diagnostics instead use `publish_raw_state(entity, bytes,
PdoType::UINT16)` (or another explicit representation). That choice defines a raw
bit-pattern presentation, not the unverified signal's physical type or units.

## Unresolved Reference Disagreements

- PDO 212 is INT16, exactly two bytes, matching the Python registry.
  A targeted Q600 KD read on 2026-10-14 returned `E9 00` (two bytes, 233), matching
  its displayed 23.3 C value. This confirms the two-byte width on that unit, not
  every hardware variant or the signedness of a positive-only sample.
  The documentation's UINT8 entry contradicts the observed width and is recorded
  as an error, not accommodated by a shim. If independent evidence establishes
  UINT16 instead, change the one runtime type and correct the documentation.
- PDO 213/216 keep their existing raw-watts interpretation and YAML conversion
  to kW. Documentation examples implying /100 are recorded, not silently applied.
- PDO 69 has no documented type and no registry definition. No type is invented.
- Undocumented PDOs, including pressure-discovery candidates, are not assigned
  meanings or types by this reference import.

## Precision and Coverage

ESPHome numeric states are floats. UINT32/INT64 values may lose precision when
published as numeric sensors; exact text/bitset processing must use integer
values before publication. The duration formatter and PDO 230 constraint path
already follow this rule. A type table entry is not a claim of a reachable CAN
handler: high-ID addressing/filtering and unexposed telemetry are separate work.

## Updating the Reference

```sh
python scripts/sync_pdo_reference.py --check
python scripts/generate_pdo_map.py --check
```

Updating deliberately requires a full upstream commit SHA. The importer reads
documentation and parses the Python AST; it never executes fetched Python code.
Review changes and direct-CAN evidence before accepting a new reference:

```sh
python scripts/sync_pdo_reference.py --revision FULL_COMMIT_SHA
python scripts/generate_pdo_map.py
python -m unittest discover -s tests -p 'test_pdo_mapping.py'
python tests/test_pdo_decode.py
```

## Sensor I2C Output Is Not PDO Encoding

The owner reports SHT20 on the older PCB and SHT30 on newer machines, and
suggests SHT31 may also fit. That hardware interchangeability is not independently
established by this work. SHT3x family compatibility must not be conflated with
an SHT20-to-SHT3x drop-in: I2C addresses, commands, and raw conversion differ.
Manufacturer documents verify that they use digital I2C, but this bridge reads
Zehnder CAN messages, not those sensors' I2C registers.

- SHT3x-DIS, section 4.13 (page 14), explicitly specifies unsigned 16-bit raw
  measurement values. Temperature: `T = -45 + 175 * raw / 65535`.
  https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf
- SHT20, sections 6-6.2 (page 10), specifies two-byte, MSB-first transfers, with
  default 14-bit temperature resolution and two status bits to clear.
  Temperature: `T = -46.85 + 175.72 * raw / 65536` after clearing those bits.
  https://sensirion.com/media/documents/CCDE1377/635000A2/Sensirion_Datasheet_Humidity_Sensor_SHT20.pdf

Unsigned raw words can therefore yield negative physical temperatures after an
offset conversion. Neither negative physical values nor sensor model alone
proves the CAN field's signedness: the unit can transform or re-encode readings.
PDO 212 is a temperature-profile target, not a direct SHT sensor register.
Its signedness follows the registry declaration; only its width is independently
observed on KD. Do not auto-detect or guess type changes from positive samples.
