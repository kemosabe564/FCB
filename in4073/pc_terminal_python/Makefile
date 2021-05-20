PYTHON_VENV = venv
PYTHON_EXE = $(PYTHON_VENV)/bin/python
PIP_EXE = $(PYTHON_VENV)/bin/pip
ACTIVATE_FILE = $(PYTHON_VENV)/bin/activate
REQUIREMENTS_FILE = requirements.txt
PYTHON_VER = python3

venv-remove:
	@if [ -d "$(PYTHON_VENV)" ]; then \
		rm -rf $(PYTHON_VENV); \
	fi

venv-reset: venv-remove venv-setup

venv-setup:
	@if [ ! -d "$(PYTHON_VENV)" ]; then \
		sudo apt install -y python3 python3-venv; \
		$(PYTHON_VER) -m venv $(PYTHON_VENV); \
		make venv-deps-install; \
	fi

venv-deps-install:
	$(PIP_EXE) install -r $(REQUIREMENTS_FILE)

venv-deps-store:
	@if [ -d "$(PYTHON_VENV)" ]; then \
		if [ -f "$(REQUIREMENTS_FILE)" ]; then \
			rm $(REQUIREMENTS_FILE); \
		fi; \
		$(PIP_EXE) freeze > $(REQUIREMENTS_FILE); \
	else \
		echo "No virtual environment installed"; \
	fi

venv-deps-add: venv-setup
	@if [ ! -z "$(PACKAGE)" ]; then \
		$(PIP_EXE) install $(PACKAGE); \
		make venv-deps-store; \
	else \
		echo "Usage: make venv-deps-add PACKAGE={name}"; \
	fi

run: venv-setup
	@$(PYTHON_EXE) ./pc_terminal.py

run-headless: venv-setup
	@$(PYTHON_EXE) ./pc_terminal.py --no-gui
