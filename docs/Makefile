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

.PHONY: check-links

check-links:
	@echo "🔎 Scanning for bad toctree references (e.g. 'design/index')..."
	@grep -r --include="*.rst" -E "\.\. toctree::|^\s+.*\/index$$" source/ || true
	@echo ""
	@echo "✅ Tip: Replace paths like 'design/index' with just 'design' in toctrees."
