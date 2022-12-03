import abc
import os
from pathlib import Path
from shutil import copy2
from typing import Generator, List

import setuptools.command.sdist


class GitSdistABC(abc.ABC, setuptools.command.sdist.sdist):
    """
    This class defines a custom command to build source distribution. It covers the case
    where projects store the setup.cfg file in a subfolder and are git repositories.

    The problem in this setup is that only the subfolder by default is packaged
    in the sdist, resulting in a archive that only contains part of the whole repo.
    """

    def make_release_tree(self, base_dir, files) -> None:
        """
        This method is the responsible of building the list of files that are included
        in the source distribution.

        These files (that could be the entire git tree) are copied in the subfolder
        before creating the archive, resulting in a source distribution like if the
        setup.cfg would be part of the top-level folder.

        This method prepares the egg metadata for the source distribution and allows
        specifying a list of files that are copied in the location of the setup.cfg.
        """

        import setuptools_scm.integration

        # Build the setuptools_scm configuration, containing useful info for the sdist
        config: setuptools_scm.integration.Configuration = (
            setuptools_scm.integration.Configuration.from_file(
                dist_name=self.distribution.metadata.name
            )
        )

        # Get the root of the git repository
        repo_root = config.absolute_root

        if not Path(repo_root).exists() or not Path(repo_root).is_dir():
            raise RuntimeError(f"Failed to find a git repo in {repo_root}")

        # Prepare the release tree by calling the original method
        super(GitSdistABC, self).make_release_tree(base_dir=base_dir, files=files)

        # Collect all the files and copy them in the subfolder containing setup.cfg
        for file in self.get_sdist_files(repo_root=repo_root):

            src = Path(file)
            dst = Path(base_dir) / Path(file).relative_to(repo_root)

            # Make sure that the parent directory of the destination exists
            dst.absolute().parent.mkdir(parents=True, exist_ok=True)

            print(f"{Path(file).relative_to(repo_root)} -> {dst}")
            copy2(src=src, dst=dst)

        # Create the updated list of files included in the sdist
        all_files_gen = Path(base_dir).glob(pattern="**/*")
        all_files = [str(f.relative_to(base_dir)) for f in all_files_gen]

        # Find the SOURCES.txt file
        sources_txt_list = list(Path(base_dir).glob(pattern=f"*.egg-info/SOURCES.txt"))
        assert len(sources_txt_list) == 1

        # Update the SOURCES.txt files with the real content of the sdist
        os.unlink(sources_txt_list[0])
        with open(file=sources_txt_list[0], mode="w") as f:
            f.write("\n".join([str(f) for f in all_files]))

    @staticmethod
    @abc.abstractmethod
    def get_sdist_files(repo_root: str) -> List[Path]:
        """
        Return all files that will be included in the source distribution.

        Note: the egg metadata are included by default, there's not need to specify them.

        Args:
            repo_root: The path to the root of the git repository.

        Returns:
            The list of files to include in the source distribution.
        """


class GitSdistTree(GitSdistABC):
    """
    This class defines a custom command to build source distribution. It covers the case
    where projects store the setup.cfg file in a subfolder and are git repositories.

    The problem in this setup is that only the subfolder by default is packaged
    in the sdist, resulting in a archive that only contains part of the whole repo.

    In particular, this class copies all the files that are part of the last git commit.
    Any uncommitted or staged files are ignored and are not part of the sdist.
    """

    @staticmethod
    def get_sdist_files(repo_root: str) -> List[Path]:

        import git

        # Create the git Repo object
        git_repo = git.Repo(path=repo_root)

        # Get all the files of the git repo recursively
        def list_paths(
            tree: git.Tree, path: Path = Path(".")
        ) -> Generator[Path, None, None]:

            for blob in tree.blobs:
                yield path / blob.name

            for tree in tree.trees:
                yield from list_paths(tree=tree, path=path / tree.name)

        # Return the list of absolute paths to all the git repo files
        return list(list_paths(tree=git_repo.commit().tree, path=Path(repo_root)))


class GitSdistFolder(GitSdistABC):
    """
    This class defines a custom command to build source distribution. It covers the case
    where projects store the setup.cfg file in a subfolder and are git repositories.

    The problem in this setup is that only the subfolder by default is packaged
    in the sdist, resulting in a archive that only contains part of the whole repo.

    In particular, this class copies all the files that are part of the git folder.
    It includes also all uncommitted and staged files.
    """

    @staticmethod
    def get_sdist_files(repo_root: str) -> List[Path]:

        # Create the list of files of the git folder
        all_files_gen = Path(repo_root).glob(pattern="**/*")

        # Return the list of absolute paths to all the git folder files (also uncommited)
        return [f for f in all_files_gen if not f.is_dir() and ".git" not in f.parts]