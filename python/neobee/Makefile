CHECKFILES = neobee

help:
	@echo  "NeoBee platform development makefile"
	@echo
	@echo  "usage: make <target>"
	@echo  "Targets:"
	@echo  "    up      Updates dev/test dependencies"
	@echo  "    deps    Ensure dev/test dependencies are installed for development"
	@echo  "    lint	Reports all linter violations"

up:
	@pip install -q --upgrade pip-tools
	@pip install -q --upgrade wheel
	@pip install -q --upgrade setuptools
	CUSTOM_COMPILE_COMMAND="make up" pip-compile -o requirements.txt requirements.in -U
	CUSTOM_COMPILE_COMMAND="make up" pip-compile -o requirements-dev.txt requirements-dev.in -U

deps:
	@pip install -q --upgrade pip-tools
	@pip install -q --upgrade wheel
	@pip install -q --upgrade setuptools
	@pip-sync requirements-dev.txt

lint: deps
	pylint $(CHECKFILES)

ci:	check
	pylint --errors-only $(CHECKFILES)
	nose2 -v $(CHECKFILES)

nose: deps
	nose2 -v $(CHECKFILES)

check: deps
	black -l 100 --check $(CHECKFILES)

style:
	black -l 100 $(CHECKFILES) 

build: ci
	rm -fR dist/
	python setup.py sdist bdist_wheel

publish: build
	twine check ./dist/*
	twine upload ./dist/*
