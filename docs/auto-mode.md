# Returning to Auto

An explicit Auto Ventilation ON now sends `85 15 08 01` to cancel manual mode,
then `85 15 01 01` to cancel the temporary fan-speed override. Cancelling only
manual mode can leave the selected fan level active until its timer expires.
Source: aiocomfoconnect v0.3.0, upstream PR #88.

The existing Boost and Manual (limited) workarounds/delays remain. OFF still only
enables manual mode. Received fan/mode state does not invoke these commands.
`make test` checks the immediate and delayed sequences, explicit fan controls,
and read-only feedback. The missing speed cancellation fails before the patch.

Hardware acceptance: select Low/High, enable Auto, and capture both cancellation
frames; verify that the unit returns to its schedule rather than retaining the
override. Repeat from Manual (limited), Boost, and Boost (RF), recording unit
model and firmware. Host tests prove commands, not physical schedule recovery.
