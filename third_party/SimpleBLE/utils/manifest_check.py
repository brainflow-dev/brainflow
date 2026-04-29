import os
from pathlib import Path
import sys
import fnmatch
try:
    from pathspec import PathSpec
    from pathspec.patterns import GitWildMatchPattern
except ImportError:
    print("Please install pathspec: pip install pathspec")
    raise

# Folders to completely ignore during search
IGNORED_FOLDERS = {
    '.git',
    '__pycache__',
    '.pytest_cache',
    '*.egg-info',
    'build',
    'dist',
    '.tox',
    '.venv',
    'venv',
    'node_modules',
    '.idea',
    '.vscode',
    '.ruff_cache',
    "simpledroidble",
    "simpledroidbridge",
    "simplejavable",
    "simplersble",
    "simplecble",
    "scripts",
    ".github",
    "docs",
    "examples",
    "hitl",
    "utils",
}

# Directories to explicitly prune in MANIFEST.in
PRUNE_DIRS = {
    ".github",
    "docs",
    "examples",
    "hitl",
    "simpleble/test",
    "simplebluez/test",
    "simpledbus/test",
    "simplepyble/test",
    "simpledroidbridge",
    "simplejavable",
    "simplersble",
    "utils",
}

# Files to explicitly exclude in MANIFEST.in
EXCLUDE_FILES = {
    "MANIFEST.in",
    ".clang-format",
    ".gitignore",
}

def generate_manifest():
    # Read .gitignore patterns
    gitignore = Path('.gitignore')
    if gitignore.exists():
        with open(gitignore, 'r') as f:
            gitignore_content = f.read().splitlines()
        spec = PathSpec.from_lines(GitWildMatchPattern, gitignore_content)
    else:
        spec = PathSpec([])

    # Find all files in project
    all_files = set()
    for root, dirs, files in os.walk('.'):
        # Convert root to use forward slashes and remove leading ./
        clean_root = root.replace('\\', '/').replace('./', '', 1)
        
        # Skip ignored folders by checking each component of the path
        root_parts = Path(clean_root).parts
        should_skip = False
        for part in root_parts:
            if any(fnmatch.fnmatch(part, ignored) for ignored in IGNORED_FOLDERS):
                should_skip = True
                break
        if should_skip:
            continue

        # Skip files in sub-test directories that are pruned
        if any(clean_root.startswith(pdir) for pdir in PRUNE_DIRS):
            continue

        for file in files:
            path = os.path.join(root, file)
            # Convert path to use forward slashes and remove leading ./
            path = path.replace('\\', '/').replace('./', '', 1)

            # Skip files that match gitignore patterns
            if spec.match_file(path):
                continue

            # Skip common file types that shouldn't be included
            if file.endswith(('.pyc', '.pyo', '.pyd')):
                continue
            
            # Skip files in EXCLUDE_FILES (they will be added as exclude directives)
            if path in EXCLUDE_FILES:
                continue

            all_files.add(path)

    # Prepare manifest content
    lines = []
    
    # Sort included files alphabetically
    for file in sorted(all_files):
        lines.append(f"include {file}")
    
    # Add prune directives
    for pdir in sorted(PRUNE_DIRS):
        lines.append(f"prune {pdir}")
        
    # Add exclude directives
    for efile in sorted(EXCLUDE_FILES):
        lines.append(f"exclude {efile}")

    # Write to MANIFEST.in
    with open('MANIFEST.in', 'w') as f:
        f.write('\n'.join(lines) + '\n')

    print("MANIFEST.in has been recreated successfully.")

if __name__ == "__main__":
    generate_manifest()
