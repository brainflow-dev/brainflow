# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.1] - 2021-XX-XX

### Fixed
- (Windows) Proper cleanup of callbacks during destruction.
- (Windows) Async timeout reduced to 10 seconds.

## [0.1.0] - 2021-12-28

### Changed
- Referenced specific version of SimpleBluez to avoid breaking changes as those libraries evolve.
- (Linux) When `scan_stop` is called, it is now guaranteed that no more scan results will be received.
- Updated Linux implementation to use SimpleBluez v0.1.1.

### Fixed
- (Linux) Scan will never stop sleeping.

## [0.0.2] - 2021-10-09

### Added
- Safe implementation of `SimpleBLE::Adapter` and `SimpleBLE::Peripheral` classes.
- CppCheck and ClangFormat CI checks. _(Thanks Andrey1994!)_
- C-style API with examples.
- Access to manufacturer data in the `SimpleBLE::Peripheral` class, for Windows and MacOS.

### Fixed
- Compilation errors that came up during development. _(Thanks fidoriel!)_
- WinRT buffer allocation would fail. _(Thanks PatrykSajdok!)_
- `SimpleBLE::Adapter` would fail to stop scanning. _(Thanks PatrykSajdok!)_
- Switched WinRT initialization to single-threaded.

### Changed
- SimpleBluez dependency migrated to OpenBluetoothToolbox.

## [0.0.1] - 2021-09-06

### Added
- Initial definition of the full API.
- Usage examples of the library.
