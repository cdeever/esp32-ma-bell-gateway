# Ma Bell Gateway - Root Makefile
# Provides convenient targets for documentation and firmware tasks

.PHONY: help docs-html docs-watch docs-clean

# Default target: show help
help:
	@echo "Ma Bell Gateway - Available Targets"
	@echo "===================================="
	@echo ""
	@echo "Setup:"
	@echo "  pip install -r docs/requirements.txt  - Install documentation dependencies"
	@echo ""
	@echo "Documentation:"
	@echo "  make html       - Build docs and start local web server (http://localhost:8000)"
	@echo "  make docs       - Build HTML documentation only"
	@echo "  make watch      - Auto-rebuild docs on file changes (live reload)"
	@echo "  make docs-clean - Clean documentation build artifacts"
	@echo ""
	@echo "Publishing:"
	@echo "  Docs are automatically published to GitHub Pages when pushed to main branch"
	@echo ""
	@echo "Firmware (ESP-IDF):"
	@echo "  idf.py build   - Build firmware"
	@echo "  idf.py flash   - Flash firmware to ESP32"
	@echo "  idf.py monitor - Monitor serial output"
	@echo ""

# Build HTML docs and start web server (main use case)
html: docs-html
	@echo ""
	@echo "Documentation built successfully!"
	@echo "Starting web server at http://localhost:8000"
	@echo "Press Ctrl+C to stop..."
	@echo ""
	@cd docs/build/html && python3 -m http.server 8000

# Build HTML docs only (no server)
docs: docs-html

docs-html:
	@echo "Building documentation..."
	@$(MAKE) -C docs html

# Auto-rebuild on file changes with live reload
watch: docs-watch

docs-watch:
	@echo "Starting sphinx-autobuild with live reload..."
	@echo "Documentation will be available at http://127.0.0.1:8000"
	@echo "Changes to .rst files will trigger automatic rebuild"
	@echo ""
	@cd docs && sphinx-autobuild source build/html

# Clean documentation build artifacts
docs-clean:
	@echo "Cleaning documentation build artifacts..."
	@$(MAKE) -C docs clean
