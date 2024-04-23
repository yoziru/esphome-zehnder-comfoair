.DEFAULT_GOAL := help
compile: .esphome/build/comfoair/.pioenvs/comfoair/firmware.bin  ## Read the configuration and compile the binary.

.esphome/build/comfoair/.pioenvs/comfoair/firmware.bin: .venv/touchfile zehnder_comfoair_q.yaml components/zehnder_comfoair_q/zehnder_comfoair_q.cpp components/zehnder_comfoair_q/zehnder_comfoair_q.h packages/*.yml boards/*.yml
	. .venv/bin/activate; esphome compile zehnder_comfoair_q.yaml

upload: .esphome/build/comfoair/.pioenvs/comfoair/firmware.bin ## Validate the configuration, create a binary, upload it, and start logs.
	. .venv/bin/activate; esphome upload zehnder_comfoair_q.yaml; esphome logs zehnder_comfoair_q.yaml

logs:
	. .venv/bin/activate; esphome logs zehnder_comfoair_q.yaml

deps: .venv/touchfile ## Create the virtual environment and install the requirements.

.venv/touchfile: requirements.txt
	test -d .venv || python -m venv .venv
	. .venv/bin/activate && pip install -Ur requirements.txt
	touch .venv/touchfile

.PHONY: clean
clean: ## Remove the virtual environment and the esphome build directory
	rm -rf .venv
	rm -rf .esphome

.PHONY: help
help: ## Show help messages for make targets
	@grep -E '^[a-zA-\/Z_-]+:.*?## .*$$' $(firstword $(MAKEFILE_LIST)) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "\033[32m%-30s\033[0m %s\n", $$1, $$2}'
