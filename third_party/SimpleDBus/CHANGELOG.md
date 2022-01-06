# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2021-12-28

### Added
- Testing framework.
- Exceptions for error detection and handling.
- Proxy object to generalize path and message handling.
- Interface object with advanced features to generalize common functionality.
- Generic callback class to generalize callback functionality.

### Changed
- Log levels use now a sane naming convention.
- Default logging level can be set during build time.
- Holders containing dictionaries can now support any generic key type.
- Message types are now part of the class and not a separate enum.

### Removed
- Legacy implementations of Property and Introspection.

### Fixed
- Interface access is now thread-safe.
- Proxy access is now thread-safe.
