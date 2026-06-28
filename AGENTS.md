# AGENTS.md

This repository contains ESPHome configuration, packages, board definitions, and
custom components for Zehnder ComfoAir Q devices.

## Operating Principles

- Keep the repository usable as a normal ESPHome project and as a reusable
  package source.
- Prefer small, reviewable changes that preserve existing board variants.
- Do not commit Wi-Fi credentials, API keys, OTA passwords, device-specific
  secrets, generated firmware, or local build caches.
- Treat hardware-facing behavior carefully. Changes to CAN bus handling,
  ventilation commands, sensors, or protocol parsing should be validated with
  config/build checks before being tested on real equipment.
- Keep project automation reproducible through the Makefile and devcontainer
  rather than relying on manually installed global tools.

## Common Commands

Use the Makefile as the main entrypoint:

```bash
make help
make validate-config
make compile
make compile BOARD=m5stack-atoms3
make compile BOARD=esp32-evb
```

The Makefile reads the ESPHome version from `requirements.txt` and runs ESPHome
through `uvx`. If `uv` is missing, install it in the devcontainer or VM package
setup instead of changing the project commands.

## Devcontainer

- Keep devcontainer files under `.devcontainer/`.
- The container should include Python, `uv`, `make`, build tools, and basic
  network troubleshooting tools.
- VS Code extensions should be declared in `devcontainer.json`, especially YAML
  and C++ support.
- Validate the container with `make help`, then `make validate-config`, then a
  board-specific `make compile`.

## Secrets And Local State

- `secrets.yaml` is local-only and must not be committed.
- Start from `secrets.yaml.example` when a local secrets file is needed.
- `.esphome/`, PlatformIO build output, generated firmware, and temporary logs
  are local artifacts.
- If shared secrets are mounted from a host or VM path, use a symlink or mount
  that stays outside Git.

## ESPHome And YAML

- Preserve existing package and substitution structure unless a refactor is
  needed for the requested change.
- Keep board-specific settings in `boards/` or the existing board YAML files.
- Keep reusable configuration in `packages/`.
- When adding YAML tags or patterns that editors may not understand, update the
  devcontainer or workspace YAML settings rather than weakening validation.

## Custom Components

- Keep C++ component changes focused and compatible with the ESPHome version in
  `requirements.txt`.
- Avoid broad protocol rewrites without adding a clear validation path.
- Prefer explicit names and comments for protocol constants, frame layouts, and
  unit conversions.
- After changing component code, run at least `make validate-config` and one
  relevant `make compile`.

## Flashing And Logs

- Prefer compile-only verification in automated or remote agent work.
- Do not flash devices or start long-running logs unless the user explicitly
  asks for it and confirms the target device.
- For OTA usage, confirm the board and target host suffix before running
  `make upload` or `make logs`.
- For USB flashing, confirm where the USB device is attached and whether the
  container has access to the serial device.

## Style

- Use ASCII unless an existing file clearly uses another character set.
- Keep documentation concise and operational.
- Match the existing Makefile, YAML, Python, and C++ style.
- Do not reformat unrelated files.
- Update README or board/package docs when user-facing behavior changes.
