# Python Terminal Docs
To run the python terminal on any machine without dependency issues *virtualenv* is used.
This creates a separate instance of python local to this application with only the necessary dependencies installed.

## Running the terminal
When running the terminal for the first time, the *venv* is set up automatically.
```
make run
```

## Adding pip packages
The pip packages that are required for this application are stored in `requirements.txt`. This is used when setting up the *venv* so that all packages are automatically installed.
This way the *venv* does not need to be stored in the repository entirely.

To install new packages in the *venv* and add them to the `requirements.txt` use:
```shell
make venv-deps-add PACKAGE={name}
```

## Manually storing the dependencies
Stores dependencies in the `requirements.txt`. Normally done automatically.
```shell
make venv-deps-store
```

## Manually installing the dependencies
Installs dependencies from the `requirements.txt`. Normally done automatically on setup.
```shell
make venv-deps-install
```

## Problems with the venv?
Removing the venv entirely:
```shell
make venv-remove
```
Reinstalling the venv:
```shell
make venv-setup
```
Full reset (both remove and setup):
```shell
make venv-reset
```
