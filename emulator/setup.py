import os
import platform
from setuptools import setup, find_packages


this_directory = os.path.abspath (os.path.dirname (__file__))
with open (os.path.join (this_directory, 'README.md'), encoding = 'utf-8') as f:
    long_description = f.read ()

setup (
    name = 'brainflow_emulator',
    version = '0.0.1',
    description = 'Emulator for brainflow project',
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
    scripts = [
        # seems like scripts section doesnt work on windows
        'brainflow_emulator/cython_linux.py',
        'brainflow_emulator/cython_windows.py'
    ],
    package_data = {
        'brainflow_emulator': [
            'com0com\\cncport.inf',
            'com0com\\com0com.cat'
            'com0com\\com0com.inf'
            'com0com\\com0com.sys'
            'com0com\\comport.inf'
            'com0com\\setup.dll'
            'com0com\\setupc.exe'
        ]
    },
    zip_safe = True,
    python_requires = '>=2.7'
)
