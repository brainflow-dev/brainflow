import os
import platform
from setuptools import setup, find_packages

if platform.system () == 'Windows':
    dll_path = 'lib\\BoardController.dll'
else:
    dll_path = 'lib/libBoardController.so'

setup (
        name = 'brainflow',
        version = '1.0',
        description = 'Library to get data from OpenBCI boards',
        url = 'https://github.com/Andrey1994/brainflow',
        author = 'Andrey Parfenov',
        author_email = 'a1994ndrey@nvidia.com',
        packages = find_packages (),
        classifiers = [
            'Development Status :: 2 - Pre-Alpha',
            'Topic :: Utilities'
        ],
        install_requires = [
            'numpy', 'scipy', 'pandas'
        ],
        package_data = {
            'brainflow': [dll_path]
        },
        zip_safe = True,
        python_requires = '>=3'
    )
