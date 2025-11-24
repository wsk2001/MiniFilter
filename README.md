# TssShield: Real-time File Access Control System

This project is a Windows file system mini-filter driver and a user-mode agent designed to control file access in real-time based on a configurable whitelist of processes.

## Components

- **Driver**: A Windows filesystem mini-filter driver that monitors file open events.
- **Agent**: A user-mode service that communicates with the driver to enforce access policies.
- **Common**: Shared header files for communication between the driver and the agent.
- **Scripts**: Batch scripts for creating certificates, signing the driver, and installing/uninstalling the system.

## Prerequisites

1.  **Visual Studio 2022**: With the C++ desktop development workload installed.
2.  **Windows Driver Kit (WDK)**: For the corresponding version of Visual Studio and Windows SDK.
3.  **x64 Native Tools Command Prompt for VS 2022**: All build commands should be executed in this environment.

## Setup and Installation Guide

### 1. Enable Test Signing Mode

The driver is not signed by a trusted authority, so you must enable Windows Test Signing Mode to install it.

1.  Open Command Prompt as an Administrator.
2.  Run the following command:
    ```
    bcdedit /set testsigning on
    ```
3.  Reboot your computer.

### 2. Build the Project

1.  Open an **x64 Native Tools Command Prompt for VS 2022**.
2.  Navigate to the project's root directory.
3.  **Build the driver**: The driver must be built using the WDK's `build.exe` utility.
    ```
    cd Driver
    build /g
    cd ..
    ```
4.  **Build the agent**: The agent is a standard user-mode application and can be built with `nmake`.
    ```
    cd Agent
    nmake
    cd ..
    ```
    The compiled binaries will be located in `Driver/x64/Release` and `Agent/x64/Release`.

### 3. Troubleshooting the Build

- **`'build' is not recognized...` or `NMAKE : fatal error U1052: file 'makefile.def' not found`**:
  This error indicates that the WDK build environment is not correctly configured. The **Driver** must be compiled in a command prompt where the WDK environment variables are set. Ensure you are using the **x64 Native Tools Command Prompt for VS 2022** and that the WDK was installed correctly as an extension to Visual Studio.

### 4. Create a Test Certificate

A test certificate is required to sign the driver for testing purposes.

1.  Navigate to the `Scripts` directory.
2.  Run the `make_cert.bat` script:
    ```
    make_cert.bat
    ```
    This will create `TssShieldCert.pfx` and `TssShieldCert.cer` in the `Scripts` directory.

### 5. Sign the Driver

After building, the driver needs to be signed with the test certificate.

1.  Ensure you are in the `Scripts` directory.
2.  Run the `sign_driver.bat` script:
    ```
    sign_driver.bat
    ```
    This script will sign `TssShield.sys` and create and sign a catalog file `TssShield.cat`.

### 6. Configure Monitored Directories

Edit the `TssShield.json` file in the project root to specify which directories to monitor and which processes are whitelisted.

```json
{
    "dirs": [
        "C:\\SecretData",
        "C:\\TestData"
    ],
    "white_list": [
        "notepad.exe",
        "powershell.exe"
    ]
}
```

- `dirs`: An array of full paths to the directories to be monitored.
- `white_list`: An array of process executable names that are allowed to access files in the monitored directories.

### 7. Install the System

1.  Copy the `TssShield.json` configuration file to `C:\TssShield.json`.
2.  Open an **Administrator Command Prompt** and navigate to the `Scripts` directory.
3.  Run the installation script:
    ```
    install_driver.bat
    ```
    This script will copy files, install the agent as a Windows service, install the driver, and start the services.

## Testing

1.  Create one of the directories specified in `TssShield.json` (e.g., `C:\SecretData`).
2.  Create a text file inside this directory.
3.  **Test 1 (Allowed Access)**: Open the text file with a whitelisted application like `notepad.exe`. The file should open successfully.
4.  **Test 2 (Denied Access)**: Try to open the same file with a non-whitelisted application (e.g., `wordpad.exe`). You should receive an "Access Denied" error.
5.  Check `C:\TssShield.log` for log messages indicating allowed or denied access.

## Uninstallation

To completely remove the driver and agent from your system:

1.  Open an **Administrator Command Prompt** and navigate to the `Scripts` directory.
2.  Run the uninstallation script:
    ```
    uninstall_driver.bat
    ```
    This script will stop and remove the services and delete the associated files.

### Disable Test Signing Mode

Once you are finished testing, you can disable Test Signing Mode:

1.  Open Command Prompt as an Administrator.
2.  Run the following command:
    ```
    bcdedit /set testsigning off
    ```
3.  Reboot your computer.
