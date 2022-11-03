from pathlib import Path
from typing import List

from setuptools import Extension


class CMakeExtension(Extension):
    """
    Custom setuptools extension that configures a CMake project.

    Args:
        name: The name of the extension.
        install_prefix: The path relative to the site-package directory where the CMake
            project is installed (typically the name of the Python package).
        disable_editable: Skip this extension in editable mode.
        write_top_level_init: Create a new top-level ``__init__.py`` file in the install
            prefix and write content.
        cmake_configure_options: List of additional CMake configure options (-DBAR=FOO).
        source_dir: The location of the main CMakeLists.txt.
        cmake_build_type: The default build type of the CMake project.
        cmake_component: The name of component to install. Defaults to all components.
        cmake_depends_on: List of dependency packages containing required CMake projects.
        expose_binaries: List of binary paths to expose, relative to top-level directory.
    """

    def __init__(
        self,
        name: str,
        install_prefix: str = "",
        disable_editable: bool = False,
        write_top_level_init: str = None,
        cmake_configure_options: List[str] = (),
        source_dir: str = str(Path(".").absolute()),
        cmake_build_type: str = "Release",
        cmake_component: str = None,
        cmake_depends_on: List[str] = (),
        expose_binaries: List[str] = (),
        cmake_generator: str = "Ninja",
    ):

        super().__init__(name=name, sources=[])

        if not Path(source_dir).is_absolute():
            source_dir = str(Path(".").absolute() / source_dir)

        if not Path(source_dir).absolute().is_dir():
            raise ValueError(f"Directory '{source_dir}' does not exist")

        self.install_prefix = install_prefix
        self.cmake_build_type = cmake_build_type
        self.disable_editable = disable_editable
        self.write_top_level_init = write_top_level_init
        self.cmake_depends_on = cmake_depends_on
        self.source_dir = str(Path(source_dir).absolute())
        self.cmake_configure_options = cmake_configure_options
        self.cmake_component = cmake_component
        self.expose_binaries = expose_binaries
        self.cmake_generator = cmake_generator
