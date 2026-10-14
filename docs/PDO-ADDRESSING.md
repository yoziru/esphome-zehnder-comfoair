# PDO Addressing and Bounds

Direct-CAN PDO addressing is `(pdo << 14) | 0x40 | node`. The documented sensor
inventory reaches PDO 1304, so the original ten-bit request limit and receive
mask excluded reference entries starting at 1024.

This implementation supports eleven PDO bits (0-2047). The encoder rejects
larger IDs and invalid local-node values instead of allowing them to alias another
address. Configuration and direct runtime requests use the same supported bound.
It does not claim every possible CAN field width or every high PDO is implemented
by every ventilation-unit firmware.

The receive signature is `0x1e003fc0`, expecting `0x40`: ignore PDO bit 10 at CAN
bit 24 while retaining the remaining namespace/address checks. Heartbeats, RMI,
firmware traffic and invalid 29-bit IDs are excluded. Tests check actual package
filters as well as the encoder at 1023/1024/1304/2047 and rejection at 2048.

No new high-ID startup reads are enabled. A documented high-ID targeted read
requires a separate bounded hardware check; lack of a response is not proof of
an addressing error or support. IDs whose physical meaning is unknown remain
unlabelled. Do not derive a type, unit, or accessory presence from a zero reply.

On 2026-10-14, a bounded KD read returned a one-byte response for PDO 1304
through the updated receive filter. PDO 1024 did not respond in the observation
window. This confirms high-ID transport for 1304 on this unit, not availability
of every high ID. No high-ID startup requests were added. Normal readings and
Auto/Medium/Normal remained intact, with no captured decoding errors.
