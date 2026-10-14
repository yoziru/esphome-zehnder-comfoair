# Optional airflow constraints

Include `packages/airflow_constraints.yml` alongside the base/dashboard package.
It appends PDO 230 without replacing existing requests. For a custom bus, override
substitution `comfoair_canbus_id` (default `comfoair_canbus`). Default firmware is unchanged.

One diagnostic text entity shows exact 64-bit raw hex and stable constraint labels,
without passing bits through an ESPHome float. Bit 45 is validity: absent means
`unavailable`; validity alone means `None`. Known reasons follow aiocomfoconnect
v0.3.0's `calculate_airflow_constraints`, including CO2 bits 47–54. Grouped aliases
are not duplicated; unmapped bits are labelled explicitly. Malformed lengths leave
the last state unchanged. The sensor never controls ventilation.

`make test` checks actual package/request merging and CAN-to-text fixtures for
invalid/valid masks, FrostProtection, grouped NoiseGuard, CO2ZoneX8 and bit 63.
Hardware interpretation beyond upstream's documented mappings is not claimed.
