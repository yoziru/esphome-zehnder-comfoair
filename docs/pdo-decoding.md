# PDO decoding audit (aiocomfoconnect v0.3.0)

The CAN bindings for UINT16 PDOs 119, 120 and 192 incorrectly requested signed
decoding. Values above 32767 became negative; 40000 independently reproduces
the error. Other UINT16 bindings (121/122, 128–130, 213–218) already decode
unsigned and remain covered as sibling contracts.

Byte assembly now explicitly follows little-endian order without unaligned,
type-punned pointer reads. Signed INT16 temperatures retain negative values.
Empty and unsupported payload lengths publish nothing. Existing one-byte
mode values and text mapping behavior are unchanged.

PDO 212 has conflicting upstream evidence (PDO docs UINT8; sensors.py INT16);
leave its existing signed binding unchanged pending an actual frame capture.
PDO 81/82 duration handling is a separate concern: their numeric float path can
lose high-value/sentinel distinctions. Keep that fix separate from this PR.

`make test` exercises production CAN dispatch and raw numeric values before
display filters, then the normal startup/thermostat checks. New signedness
regressions fail before the fix. No optional entities or settings are introduced.
