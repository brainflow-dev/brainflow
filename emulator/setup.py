import os
from setuptools import setup, find_packages

setup(
    name='brainflow_emulator',
    version='0.0.1',
    description='Emulator for brainflow project',
    url='https://github.com/Andrey1994/brainflow',
    author='Andrey Parfenov',
    author_email='a1994ndrey@gmail.com',
    packages=find_packages(),
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'Topic :: Utilities'
    ],
    install_requires=[
        'pyserial'
    ],
    package_data={
        'brainflow_emulator': [
            os.path.join('com0com', 'setup_com0com_W7_x64_signed.exe')
        ]
    },
    zip_safe=True,
    python_requires='>=2.7'
)
