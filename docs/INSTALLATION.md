# MiniFilter Installation and Usage Guide

## Overview

This guide provides detailed instructions for installing, configuring, and using the MiniFilter file system mini-filter driver on Windows.

## Prerequisites

Before installing the MiniFilter driver, ensure you have:

1. **Windows Operating System**
   - Windows 7 or later (x86/x64)
   - Windows Server 2008 R2 or later
   - Windows 10/11 (recommended)

2. **Administrator Privileges**
   - Required for driver installation and management

3. **Test Signing Mode** (for unsigned drivers during development)
   - Enable test signing: `bcdedit /set testsigning on`
   - Reboot required after enabling

## Building the Driver

### Using Visual Studio with WDK

1. Install Visual Studio (2019 or later recommended)
2. Install Windows Driver Kit (WDK)
3. Open `MiniFilter.vcxproj` in Visual Studio
4. Select your target configuration (Debug/Release) and platform (x64/x86)
5. Build the solution (Ctrl+Shift+B)

### Using Command Line with WDK

```cmd
cd src
msbuild MiniFilter.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Installation

### Method 1: Using INF File (Recommended)

1. Right-click on `MiniFilter.inf` in Windows Explorer
2. Select "Install" from the context menu
3. Confirm the installation in the security prompt

### Method 2: Using Command Line

```cmd
# Navigate to the driver directory
cd src

# Install the driver
rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 .\MiniFilter.inf
```

### Method 3: Using Device Manager

1. Open Device Manager
2. Click "Action" > "Add legacy hardware"
3. Follow the wizard to install from INF file
4. Browse to `MiniFilter.inf`

## Loading the Driver

### Using Filter Manager Control (fltmc)

```cmd
# Load the driver
fltmc load MiniFilter

# Verify the driver is loaded
fltmc filters

# Check instances
fltmc instances
```

### Using Service Control (sc)

```cmd
# Start the service
sc start MiniFilter

# Query the service status
sc query MiniFilter
```

### Using net command

```cmd
# Start the service
net start MiniFilter

# Stop the service
net stop MiniFilter
```

## Configuration

### Altitude Configuration

The driver's altitude is configured in the INF file:
- Default altitude: `370020`
- Altitude range for Activity Monitor: `360000-389999`

To change the altitude:
1. Edit `MiniFilter.inf`
2. Modify the `Instance1.Altitude` value
3. Reinstall the driver

### Instance Configuration

Instances can be configured in the registry:
```
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\MiniFilter\Instances
```

## Monitoring and Debugging

### Viewing Debug Output

Use DebugView or WinDbg to view debug output:

1. Download and run [DebugView](https://docs.microsoft.com/sysinternals/downloads/debugview)
2. Enable "Capture Kernel" option
3. You should see MiniFilter debug messages

### Using WinDbg

```
# Attach WinDbg to kernel
# Set up symbol path
.sympath srv*c:\symbols*https://msdl.microsoft.com/download/symbols

# View MiniFilter debug output
!fltkd.filters
!fltkd.filter MiniFilter
```

### Event Viewer

Check the System event log for driver-related messages:
```cmd
eventvwr.msc
```

## Unloading the Driver

### Using Filter Manager Control

```cmd
# Unload the driver
fltmc unload MiniFilter
```

### Using Service Control

```cmd
# Stop the service
sc stop MiniFilter
```

## Uninstallation

### Method 1: Using INF File

```cmd
rundll32.exe setupapi,InstallHinfSection DefaultUninstall 132 .\MiniFilter.inf
```

### Method 2: Using Device Manager

1. Open Device Manager
2. Find "MiniFilter" under "File System Filter Drivers"
3. Right-click and select "Uninstall"

### Method 3: Manual Removal

```cmd
# Stop and delete the service
sc stop MiniFilter
sc delete MiniFilter

# Remove the driver file
del %SystemRoot%\System32\drivers\MiniFilter.sys
```

## Troubleshooting

### Driver Fails to Load

**Problem:** Driver returns error code when loading

**Solutions:**
1. Check if test signing is enabled: `bcdedit /enum {current}`
2. Verify the driver is properly signed
3. Check System event log for error messages
4. Ensure WDK runtime components are installed

### Cannot See Debug Output

**Problem:** No debug messages appear in DebugView

**Solutions:**
1. Verify DebugView is running with admin privileges
2. Enable "Capture Kernel" in DebugView
3. Check if DbgPrint is enabled in the system
4. Set debug print filter: `reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter" /v DEFAULT /t REG_DWORD /d 0xF`

### Driver Causes System Instability

**Problem:** System crashes or becomes unstable after loading

**Solutions:**
1. Unload the driver immediately: `fltmc unload MiniFilter`
2. Boot into Safe Mode if necessary
3. Review the minidump files in `%SystemRoot%\Minidump`
4. Analyze with WinDbg: `!analyze -v`

### Permission Denied Errors

**Problem:** Cannot install or load driver

**Solutions:**
1. Run commands with administrator privileges
2. Disable secure boot in BIOS/UEFI
3. Enable test signing mode
4. Check code integrity policies

## Performance Considerations

1. **Callback Selection**: Only register for necessary I/O operations
2. **Completion Routines**: Use post-operation callbacks sparingly
3. **Memory Usage**: Monitor kernel pool usage
4. **Logging**: Disable verbose logging in production

## Security Considerations

1. **Code Signing**: Always sign production drivers with a valid certificate
2. **Input Validation**: Validate all data from user mode
3. **Memory Safety**: Use SAL annotations and run Static Driver Verifier
4. **Least Privilege**: Request minimal permissions required

## Additional Resources

- [Windows Driver Kit Documentation](https://docs.microsoft.com/windows-hardware/drivers/)
- [File System Minifilter Drivers](https://docs.microsoft.com/windows-hardware/drivers/ifs/file-system-minifilter-drivers)
- [Filter Manager Concepts](https://docs.microsoft.com/windows-hardware/drivers/ifs/filter-manager-concepts)
- [OSR Online](https://www.osronline.com/) - Driver development community

## Support

For issues, questions, or contributions, please visit the project repository.
