import os
import platform
from setuptools import setup, find_packages


this_directory = os.path.abspath (os.path.dirname (__file__))
with open (os.path.join (this_directory, 'README.md')) as f:
    long_description = f.read ()

setup (
    name = 'brainflow',
    version = '1.0.7',
    description = 'Library to get data from OpenBCI boards',
    long_description = long_description,
    long_description_content_type = 'text/markdown',
    url = 'https://github.com/Andrey1994/brainflow',
    author = 'Andrey Parfenov',
    author_email = 'a1994ndrey@gmail.com',
    packages = find_packages (),
    classifiers = [
        'Development Status :: 2 - Pre-Alpha',
        'Topic :: Utilities'
    ],
    install_requires = [
        'numpy', 'scipy', 'pandas', 'enum-compat'
    ],
    package_data = {
        'brainflow': [
            os.path.join ('lib', 'BoardController.dll'),
            os.path.join ('lib', 'libBoardController.so'),
            os.path.join ('lib', 'GanglionLib.dll'),
            os.path.join ('lib', 'GanglionLibNative64.dll'),
            os.path.join ('lib', 'libBoardController.dylib')
        ]
    },
    zip_safe = True,
    python_requires = '>=2.7'
)
