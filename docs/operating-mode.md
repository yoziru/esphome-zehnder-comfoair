# Operating Mode

PDO 49 is the active timer of the preset schedule instance. The other values keep the
mapping in `packages/text_sensors.yml`; the manual values need one extra step:

| value | meaning |
|-------|---------|
| 1 | Manual (limited), unless manual mode is active (see below) |
| 5 | Manual (permanent) |
| 255 | Auto |

Value `1` is ambiguous by itself. Changing the fan level starts timer 1 of the preset
instance, so PDO 49 reads `1` both for a temporary override and for a level change made
while permanent manual mode is active. The unit display and the ComfoControl app keep
showing Manual (permanent) in the second case because manual mode is still on.

PDO 56 (the manual-mode instance's active timer) tells them apart: `1` means manual mode
is active, `255` means auto. Operating Mode therefore reports `Manual (permanent)` for
PDO 49 = 1 while PDO 56 = 1, and `Manual (limited)` when PDO 56 is `255` or not received
yet. Both raw values remain visible in `comfoair_dump` DEBUG logs.

Hardware acceptance: with Auto off, change the fan level and confirm Operating Mode stays
Manual (permanent) while the unit display does. A unit that does not report PDO 56 keeps
the previous Manual (limited) label for value 1.
