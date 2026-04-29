# Build Python Wheels GitHub Action

This action builds Python wheels for your package using [cibuildwheel](https://cibuildwheel.readthedocs.io/), a tool for building Python wheels for multiple platforms.

## Features

- Builds wheels for multiple platforms (Linux, macOS, Windows)
- Supports multiple Python versions and architectures
- Optional wheel publishing to TestPyPI
- Configurable build parameters

## Usage

Add this action to your workflow file:

```yaml
jobs:
  build-wheels:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]

    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          fetch-depth: 0
          fetch-tags: true

      - name: Build Python Wheels
        uses: ./.github/actions/build-python-wheels
        with:
          os: ${{ matrix.os }}
          # Optional parameters with defaults shown:
          # python-version: '3.11'
          # requirements-file: 'simplepyble/requirements.txt'
          # publish-to-testpypi: 'false'
          # cibw-build-verbosity: '3'
          # macosx-deployment-target: '13.0'
          # cibw-skip: '*musllinux_* pp* cp36-* cp37-*'
```

## Inputs

| Input                   | Description                                               | Required | Default                      |
|-------------------------|-----------------------------------------------------------|----------|------------------------------|
| `os`                    | Operating system to build wheels for                      | Yes      | -                            |
| `python-version`        | Python version to use for building                        | No       | `3.11`                       |
| `requirements-file`     | Path to requirements.txt file                             | No       | `simplepyble/requirements.txt` |
| `publish-to-testpypi`   | Whether to publish the built wheels to TestPyPI           | No       | `false`                      |
| `cibw-build-verbosity`  | Verbosity level for cibuildwheel                          | No       | `3`                          |
| `macosx-deployment-target` | Minimum macOS version to target                       | No       | `13.0`                       |
| `cibw-skip`             | Patterns for skipping certain builds                      | No       | `*musllinux_* pp* cp36-* cp37-*` |

## Outputs

| Output        | Description                                   |
|---------------|-----------------------------------------------|
| `wheels-path` | Path where the wheels are stored after building |

## Publishing to TestPyPI

To publish wheels to TestPyPI, set the `publish-to-testpypi` input to `true` and provide the TestPyPI credentials as inputs to your workflow:

```yaml
- name: Build Python Wheels
  uses: ./.github/actions/build-python-wheels
  with:
    os: ${{ matrix.os }}
    publish-to-testpypi: 'true'
  env:
    TEST_PYPI_USER: ${{ secrets.TEST_PYPI_USER }}
    TEST_PYPI_PASSWORD: ${{ secrets.TEST_PYPI_PASSWORD }}
```

## License

This project is part of the SimpleBLE repository and follows its licensing terms.