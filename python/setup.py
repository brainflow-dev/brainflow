import os
from setuptools import setup, find_packages

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
            'numpy', 'enum34'
        ],
        package_data = {
            'brainflow': [ 'lib/libBoardController.so']
        },
        zip_safe = True,
        python_requires = '>=2.7'
    )
