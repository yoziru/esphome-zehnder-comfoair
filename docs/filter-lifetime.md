# Optional filter lifetime control

Include `packages/filter_lifetime.yml` alongside the normal base/dashboard package.
The Requested Filter Lifetime number writes FILTER `1c/01/03` only when explicitly
set. Its initial 150-day display is a local suggested/requested value, not device
readback, and is never sent on boot. Restart does not reapply a setting.

Whole-day values must fit UINT16 (0–65535). This is a wire limit, not a claim that
every ventilation unit accepts that range: consult/verify the unit's allowed range
before changing it. Negative, fractional and non-finite inputs send nothing. The
number publishes a requested value after transmission, not an acknowledgement.

300 days encodes as `03 1c 01 03 2c 01`. Existing one-byte property writes are
unchanged. `make test` checks exact bytes, extremes, invalid public/UI inputs and
read-only fresh/saved-preference boots. On hardware, capture the command and verify
the actual filter-lifetime setting independently; no RMI readback is implemented.
