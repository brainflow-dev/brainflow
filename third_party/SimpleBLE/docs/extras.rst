======================
Building documentation
======================

To build documentation for the projects in this repository,
you first need to install Sphynx, using the following commands: ::

   pip3 install sphinx sphinx_rtd_theme

Once all dependencies have been installed, HTML documentation can be built
by calling the following commands: ::

   cd <path-to-repository>/docs
   make html


=================
Release checklist
=================

Before releasing a new version of the project, the following steps should be
performed:

#. Review/update the version number in the following files:

   - ``VERSION``
   - ``docs/changelog.rst``
   - ``simplepyble/setup.py``

#. Commit the changes to the repository.

#. Tag the commit with the new version number.

#. Push the commit and the tag to the remote repository.

#. Create a new release on GitHub.

#. Run the CI job to build and upload the package to PyPI.

#. Advance the version number in the following files:

   - ``VERSION``
   - ``docs/changelog.rst``
   - ``simplepyble/setup.py``
