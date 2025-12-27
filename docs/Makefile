# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)


GHPAGES_DIR=../../ma-bell-docs

publish: html
	rsync -av --delete --exclude='.git' --exclude='.gitignore' build/html/ $(GHPAGES_DIR)/
	touch $(GHPAGES_DIR)/.nojekyll
	cd $(GHPAGES_DIR) && \
		git add . && \
		git commit -m "Update docs" && \
		git push origin gh-pages

.PHONY: watch

watch:
	@echo "Starting sphinx-autobuild on http://127.0.0.1:8000"
	@echo "Press Ctrl+C to stop..."
	sphinx-autobuild source build/html

.PHONY: serve

serve: html
	@echo "Serving documentation at http://localhost:8000"
	@echo "Press Ctrl+C to stop..."
	@cd build/html && python3 -m http.server 8000
