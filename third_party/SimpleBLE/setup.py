import argparse
import os
import pathlib
import sys

import pybind11
import skbuild


def exclude_unnecessary_files(cmake_manifest):
    def is_necessary(name):
        is_necessary = (
            name.endswith(".so")
            or name.endswith(".dylib")
            or name.endswith("py")
            or name.endswith("pyi")
            or name.endswith("pyd")
            or name.endswith("py.typed")
        )
        print(f"Parsing file: {name} - {is_necessary}")
        return is_necessary

    return list(filter(is_necessary, cmake_manifest))

argparser = argparse.ArgumentParser(add_help=False)
argparser.add_argument(
    "--plain", help="Use Plain SimpleBLE", required=False, action="store_true"
)
args, unknown = argparser.parse_known_args()
sys.argv = [sys.argv[0]] + unknown

root = pathlib.Path(__file__).parent.resolve()
version_str = (root / "VERSION").read_text(encoding="utf-8").strip()

# Get the long description from the README file
long_description = (root / "simplepyble" / "README.rst").read_text(encoding="utf-8")

cmake_options = []
cmake_options.append(f"-Dpybind11_DIR={pybind11.get_cmake_dir()}")
if sys.platform == "win32":
    cmake_options.append("-DCMAKE_SYSTEM_VERSION=10.0.19041.0")
elif sys.platform.startswith("darwin"):
    cmake_options.append("-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0")
cmake_options.append(f"-DPYTHON_EXECUTABLE={sys.executable}")
cmake_options.append(f"-DSIMPLEPYBLE_VERSION={version_str}")

if args.plain:
    cmake_options.append("-DSIMPLEBLE_PLAIN=ON")

if "PIWHEELS_BUILD" in os.environ:
    cmake_options.append("-DLIBFMT_VENDORIZE=OFF")

skbuild.setup(
    name="simplepyble",
    version=version_str,
    author="Kevin Dewald",
    author_email="kevin@simpleble.org",
    url="https://github.com/simpleble/simpleble",
    description="The ultimate fully-fledged cross-platform BLE library, designed for simplicity and ease of use.",
    long_description=long_description,
    long_description_content_type="text/x-rst",
    packages=["simplepyble"],
    package_dir={"": "simplepyble/src"},
    package_data={"simplepyble": ["py.typed", "*.pyi"]},
    cmake_source_dir="simplepyble",
    cmake_args=cmake_options,
    cmake_process_manifest_hook=exclude_unnecessary_files,
    cmake_install_dir="simplepyble/src/simplepyble",
    setup_requires=[
        "setuptools>=42",
        "scikit-build",
        "ninja; platform_system!='Windows'",
        "cmake>=3.21",
        "pybind11",
    ],
    install_requires=[],
    extras_require={},
    platforms="Windows, macOS, Linux",
    python_requires=">=3.8",
    classifiers=[
        "License :: Free for non-commercial use",
        "License :: Other/Proprietary License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: Python :: 3.14",
        "Programming Language :: Python :: 3 :: Only",
    ],
)
