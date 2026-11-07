# PDO Type Mapping

`components/zehnder_comfoair_q/pdo_types.h` is the hand-maintained, sorted table
of declared PDO types. Its reference revision is recorded in the header. Each
known PDO has one type and an exact payload width; unknown IDs, wrong widths,
and invalid BOOL values are rejected without replacing the last valid state.
Table entries do not enable new handlers, requests, or sensors.

`comfoair_on_frame_case(pdo, component, entity)` takes three arguments.
Custom handlers can use `publish_pdo_state(entity, bytes, pdo)` directly. The old
fourth signedness argument and length-inferred publishing overload are removed;
update custom lambdas rather than adding compatibility paths. Integer decoding
stays exact until numeric publication; timers and constraint bitsets use integer
values before formatting instead of interpreting a float as a sentinel/bitset.

Reference disagreements are noted beside the type table:
- PDO 212 follows the registry's two-byte INT16 type rather than documented UINT8;
  the observed width alone does not independently establish signedness.
- PDOs 213/216 retain existing raw-watts scaling; conflicting documentation examples
  do not justify an unverified scale change.
- PDO 69 has no declared reference type and is omitted.

Run `make test` for decoder boundaries, exact-width rejection, table lookup, and
the real ESPHome publication path. Existing sanitizer and iterator-bounds jobs
exercise the same checks.
