# CAN subsystem test audit

Scope: production `ZehnderComfoAirQ` public methods, CAN dispatch and YAML entity automations; ESPHome 2026.9.1's actual host entities, scheduler, restore behavior and triggers. `68a14b6` is the starting owner boundary. Before that commit this repository had configuration validation/firmware CI but no executable C++ regressions.

| Contract | Primary owner / credible regression |
| --- | --- |
| Read-only boot with and without climate, saved preferences | `startup_checks.cpp` setup frame assertion; catches switch restore and thermostat startup writes |
| Text sensor polling does not invent/reset a profile | Direct real template update before/after PDO; independent check of OP's workaround hypothesis |
| Profile sensor/select reflect a CAN PDO | CAN-to-entity assertions; detects routing/filter/feedback mistakes |
| Fan PDOs update state without outbound controls | Initial nonzero PDO, all four levels, both mode orders; catches ON-trigger echo and reordering bugs |
| Explicit controls remain functional | Literal transmitted timer bytes for selects/switches/fan; prevents a blanket startup guard from swallowing commands |
| No deferred feedback command survives | Real scheduler delay assertion; uniquely reaches delayed automations |
| Thermostat remains functional after synchronization | Real temperature PDOs, modes and minimum timers; protects idle guard without private trigger tests |
| Command framing and sequence | Added public-method/frame tests; no existing lifecycle case reaches 8/9/14/15-byte boundaries or full rollover |
| RTR request ordering/non-overlap/restart/empty list | Added real-scheduler tests; existing command capture intentionally excluded RTRs |

Retain the existing cases. Repeated fan cases vary actual level/transition/order rather than duplicating an implementation predicate. The fake only owns physical I/O and does not implement ESPHome state, callbacks, timing or restore. No tests or production seams were removed: there is no evidence that deleting them improves confidence. No test-only production export is added.

Remaining risks belong in their fix/feature PRs: unsigned PDO bindings, safe byte reads, full 64-bit constraints, typed property writes and Auto override cancellation. Regression cases must fail before their fix for the intended observable reason. Do not land known product failures in this test-only branch.

Run `make test`. Set `ESPHOME_TEST_SANITIZE=1 make test` to build/run the real host code under ASan/UBSan. This checks executed paths, not every iterator precondition or strict-aliasing rule. Remote CI and physical ventilation-unit behavior are not claimed by a local pass.
