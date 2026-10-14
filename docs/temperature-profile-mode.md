# Optional temperature profile mode

Include `packages/temperature_profile_mode.yml` alongside the normal package.
Requested Temperature Profile Mode offers Adaptive (`03 1d 01 08 00`) and Fixed
(`03 1d 01 08 01`). It is distinct from the existing Normal/Cool/Warm profile
selection (a schedule timer), and does not change that selection as a side effect.

The initial Adaptive display and later selected values are requested/assumed,
not confirmed readback. The unit is written only by an explicit user selection;
setup, state feedback and restart do not apply settings. The control is opt-in
and does not restore local preferences. No external-setpoint option is offered
because upstream marks its meaning uncertain.

`make test` checks exact property bytes, invalid-option rejection, independence
from profile PDOs and read-only fresh/saved-preference boots. On hardware, verify
the actual configured mode independently after the request; no ACK/readback is
implemented and host tests do not prove that a particular firmware accepts it.
