# MiniFilter Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-11-24

### Added
- Initial release of MiniFilter driver
- Basic file system monitoring capabilities
- IRP_MJ_CREATE operation monitoring (pre and post callbacks)
- IRP_MJ_READ operation monitoring (pre callback)
- IRP_MJ_WRITE operation monitoring (pre callback)
- Debug logging for all monitored operations
- Driver installation and uninstallation support via INF file
- Instance setup and teardown handling
- Filter Manager integration
- Visual Studio project configuration
- Support for x86, x64, ARM, and ARM64 architectures
- Comprehensive documentation:
  - Installation guide
  - Development guide
  - API reference
- Build system support:
  - Visual Studio project file
  - Makefile for WDK build environment
- MIT License

### Features
- Kernel-mode operation for maximum performance
- Low overhead monitoring
- Altitude-based filtering (370020 in Activity Monitor range)
- Compatible with Windows 7 and later
- Support for multiple file system types (NTFS, FAT32, exFAT, etc.)
- Clean driver unload support

### Documentation
- README with quick start guide (Korean and English)
- Detailed installation instructions
- Developer guide with architecture overview
- API reference documentation
- Code examples and best practices

### Build System
- Visual Studio project with multiple configurations
- Support for Debug and Release builds
- Multi-architecture support
- Integration with Windows Driver Kit (WDK)

## [Unreleased]

### Planned Features
- User-mode communication interface
- Configuration file support
- Enhanced logging options
- File operation statistics
- Selective filtering based on file paths
- Process-based filtering
- Performance optimizations
- Additional IRP operation monitoring
- Context management examples
- More comprehensive error handling

---

## Version History

### Version 1.0.0 (2025-11-24)
- Initial public release
- Core functionality implemented
- Basic monitoring capabilities
- Documentation complete

---

## Notes

This is the initial release of the MiniFilter driver project. Future versions will add more advanced features based on user feedback and requirements.

For issues or feature requests, please visit the project repository on GitHub.
