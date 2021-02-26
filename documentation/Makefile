# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build
CHECKFILES    = neobee

up:
	@pip install -q --upgrade pip-tools
	@pip install -q --upgrade wheel
	CUSTOM_COMPILE_COMMAND="make up" pip-compile -o requirements.txt requirements.in -U

deps:
	@pip install -q --upgrade pip-tools
	@pip install -q --upgrade wheel
	@pip-sync requirements.txt

checkdocs:
	doc8 $(SOURCEDIR)

docs:
	sphinx-build -E -b html source build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
