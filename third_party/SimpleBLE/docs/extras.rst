Extras
======

Building documentation
----------------------


To build documentation for the projects in this repository,
you first need to install Sphynx, using the following commands: ::

   pip3 install -r docs/requirements.txt

Once all dependencies have been installed, HTML documentation can be built
by calling the following commands: ::

   cd <path-to-repository>/docs
   make html

Release checklist
-----------------

Before releasing a new version of the project, the following steps should be
performed:

#. Ensure content parity between all readmes and the documentation.

   - ``README.rst``
   - ``LICENSE.md``
   - ``simplepyble/README.rst``
   - ``simplersble/README.md``

#. Review/update the version number in the following files:

   - ``VERSION``
   - ``Cargo.toml``
   - ``docs/changelog.rst``
   - ``setup.py``

#. Commit the changes to the repository.

#. Tag the commit with the new version number.

#. Push the commit and the tag to the remote repository.

#. Create a new release on GitHub.

#. Wait for ci_release.yml to finish.

#. Advance the version number in the following files:

   - ``VERSION``
   - ``Cargo.toml``
   - ``docs/changelog.rst``