# Project-specific configuration
PROJECT := zehnder-comfoair-q
# Default board
BOARD := m5stack-atoms3

include Makefile.common

.PHONY: test-publish
test-publish: ## Test board-specific web installer manifests
	python3 -m unittest discover -s tests -p 'test_publish*.py' -v
