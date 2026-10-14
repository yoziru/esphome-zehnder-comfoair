# Project-specific configuration
PROJECT := zehnder-comfoair-q
# Default board
BOARD := m5stack-atoms3

include Makefile.common

.PHONY: test
test: check-uv ## Run host C++ startup/feedback regression tests (no device needed).
	uv run --with esphome==$(ESPHOME_VERSION) python tests/test_startup.py
