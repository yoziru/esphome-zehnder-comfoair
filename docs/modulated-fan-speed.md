# Optional modulated fan speed

Include `packages/modulated_fan_speed.yml` alongside the normal base/dashboard
package. It appends PDO 226 to existing requests and adds one diagnostic entity;
no default package includes it. If using a custom CAN ID, set substitution
`comfoair_canbus_id` to that bus's ESPHome entity ID (default `comfoair_canbus`).

The raw UINT16 value typically ranges from 0 to 300 while PDO 225 reports
overruling; it is not the four-level selector. No unverified unit or scaling is
assigned. Feedback never changes the fan or sends control frames.

`make test` checks actual package merging, unsigned values, malformed frames,
read-only feedback, and the existing startup/thermostat contracts.
