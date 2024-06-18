import os
from contextlib import contextmanager
from pathlib import Path

from . import build_ext_option, sdist_command
from .build_extension import BuildExtension
from .cmake_extension import CMakeExtension
from .sdist_command import GitSdistFolder, GitSdistTree


@contextmanager
def build_extension_env():
    """
    Creates a context in which build extensions can be imported.

    It fixes a change of behaviour of Python >= 3.8 in Windows:
    https://docs.python.org/3/whatsnew/3.8.html#bpo-36085-whatsnew

    Other related resources:

    - https://stackoverflow.com/a/23805306
    - https://www.mail-archive.com/dev@subversion.apache.org/msg40414.html

    Example:

    .. code-block:: python

        from cmake_build_extension import build_extension_env

        with build_extension_env():
            from . import bindings
    """

    cookies = []

    # Windows and Python >= 3.8
    if hasattr(os, "add_dll_directory"):

        for path in os.environ.get("PATH", "").split(os.pathsep):

            if path and Path(path).is_absolute() and Path(path).is_dir():
                cookies.append(os.add_dll_directory(path))

    try:
        yield

    finally:
        for cookie in cookies:
            cookie.close()