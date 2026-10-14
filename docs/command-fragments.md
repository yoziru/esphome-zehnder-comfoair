# Command fragment iterator bounds

The old seven-byte iterator loop formed `command_pos + 7` past the end of a short
last fragment, then advanced past the end again. This violates vector iterator
preconditions even when the subsequent `min` happens to produce the right bytes.
ASan/UBSan alone did not catch it. A nine-byte command aborts with libc++ bounded
vector iterators before the fix.

The loop now bounds each fragment's length using byte offsets before constructing
iterators. Frame bytes, indices, final marker and sequence are unchanged.

Run `ESPHOME_TEST_BOUNDS=1 make test`: macOS uses libc++ bounded vector iterators
and debug hardening; Linux uses libstdc++ `_GLIBCXX_DEBUG`. CI adds this test mode
to the existing job. Normal and ASan/UBSan runs remain separate. Compiler-library
support is required; the macOS bounded mode requires sufficiently recent libc++.
Remote CI results are not implied by local validation.

This PR depends on `test/comfoair-can-contracts` for its owning 8/9/14/15-byte
framing tests, not on any unrelated fix or optional feature.
