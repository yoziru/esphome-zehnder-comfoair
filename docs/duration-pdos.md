# Next-change durations

PDOs 81/82 are UINT32 seconds; `ff ff ff ff` means no scheduled change. Previously
signed decoding treated large durations as negative, and formatting through an
ESPHome numeric float lost the distinction between the sentinel and nearby values.

Formatting now uses the exact four-byte unsigned value before any float conversion.
The existing raw numeric entities/filters remain, but their float state is inherently
approximate for large integers. Text values retain exact seconds; unsupported lengths
leave the last value unchanged. Negative direct helper inputs still format as `n/a`.

`make test` proves the all-ones sentinel, 2147483648 and 4294967294 seconds, malformed
frames and read-only feedback. Large-duration regressions fail before the fix.
