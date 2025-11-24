# MiniFilter Examples

This document provides practical examples of using and extending the MiniFilter driver.

## Table of Contents
- [Basic Usage](#basic-usage)
- [Advanced Operations](#advanced-operations)
- [Extending the Driver](#extending-the-driver)
- [Testing](#testing)

## Basic Usage

### Example 1: Loading and Testing the Driver

```batch
@echo off
REM load_driver.bat - Script to load MiniFilter driver

echo Loading MiniFilter driver...
fltmc load MiniFilter

if %ERRORLEVEL% EQU 0 (
    echo Driver loaded successfully!
    echo.
    echo Checking driver status...
    fltmc filters | findstr MiniFilter
    echo.
    echo Checking instances...
    fltmc instances | findstr MiniFilter
) else (
    echo Failed to load driver. Error: %ERRORLEVEL%
    echo Please check:
    echo - Test signing is enabled
    echo - Driver is properly installed
    echo - Running with admin privileges
)

pause
```

### Example 2: Monitoring File Operations

```batch
@echo off
REM test_operations.bat - Test file operations with monitoring

echo Starting file operation test...
echo.

REM Create a test file
echo Test Content > test_file.txt
echo Created test_file.txt

REM Read the file
type test_file.txt > nul
echo Read test_file.txt

REM Append to the file
echo More Content >> test_file.txt
echo Wrote to test_file.txt

REM Copy the file
copy test_file.txt test_file_copy.txt > nul
echo Copied test_file.txt

REM Delete files
del test_file.txt test_file_copy.txt
echo Deleted test files

echo.
echo Check DebugView for MiniFilter log messages!
pause
```

### Example 3: PowerShell Monitoring Script

```powershell
# monitor.ps1 - Monitor MiniFilter with PowerShell

# Load the driver
Write-Host "Loading MiniFilter..." -ForegroundColor Green
fltmc load MiniFilter

if ($LASTEXITCODE -eq 0) {
    Write-Host "Driver loaded successfully!" -ForegroundColor Green
    
    # Display filter information
    Write-Host "`nFilter Information:" -ForegroundColor Yellow
    fltmc filters | Select-String "MiniFilter"
    
    # Display instances
    Write-Host "`nInstances:" -ForegroundColor Yellow
    fltmc instances | Select-String "MiniFilter"
    
    # Perform test operations
    Write-Host "`nPerforming test operations..." -ForegroundColor Green
    
    # Create test directory
    $testDir = "C:\MiniFilterTest"
    if (!(Test-Path $testDir)) {
        New-Item -ItemType Directory -Path $testDir | Out-Null
        Write-Host "Created test directory: $testDir"
    }
    
    # Test file operations
    $testFile = Join-Path $testDir "test.txt"
    
    "Test content" | Out-File $testFile
    Write-Host "Created file: $testFile"
    
    Get-Content $testFile | Out-Null
    Write-Host "Read file: $testFile"
    
    "Additional content" | Add-Content $testFile
    Write-Host "Appended to file: $testFile"
    
    Remove-Item $testFile
    Write-Host "Deleted file: $testFile"
    
    Remove-Item $testDir
    Write-Host "Removed test directory"
    
    Write-Host "`nCheck DebugView for detailed logs!" -ForegroundColor Cyan
} else {
    Write-Host "Failed to load driver!" -ForegroundColor Red
}
```

## Advanced Operations

### Example 4: Getting File Names in Callbacks

```c
// Add this to MiniFilterPreCreate function

FLT_PREOP_CALLBACK_STATUS
MiniFilterPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION nameInfo = NULL;
    
    UNREFERENCED_PARAMETER(CompletionContext);
    
    // Get the file name
    status = FltGetFileNameInformation(
        Data,
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
        &nameInfo
    );
    
    if (NT_SUCCESS(status)) {
        // Parse the file name
        status = FltParseFileNameInformation(nameInfo);
        
        if (NT_SUCCESS(status)) {
            // Log the operation
            DbgPrint("MiniFilter: PreCreate - File: %wZ\n", &nameInfo->Name);
            DbgPrint("MiniFilter: PreCreate - Volume: %wZ\n", &nameInfo->Volume);
            DbgPrint("MiniFilter: PreCreate - Share: %wZ\n", &nameInfo->Share);
            DbgPrint("MiniFilter: PreCreate - Extension: %wZ\n", &nameInfo->Extension);
        }
        
        FltReleaseFileNameInformation(nameInfo);
    }
    
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}
```

### Example 5: Filtering Based on File Extension

```c
// Check if file has specific extension
BOOLEAN IsTargetExtension(
    _In_ PUNICODE_STRING Extension
)
{
    UNICODE_STRING txtExt;
    UNICODE_STRING docExt;
    UNICODE_STRING pdfExt;
    
    RtlInitUnicodeString(&txtExt, L"txt");
    RtlInitUnicodeString(&docExt, L"doc");
    RtlInitUnicodeString(&pdfExt, L"pdf");
    
    return (RtlEqualUnicodeString(Extension, &txtExt, TRUE) ||
            RtlEqualUnicodeString(Extension, &docExt, TRUE) ||
            RtlEqualUnicodeString(Extension, &pdfExt, TRUE));
}

// Use in PreWrite callback
FLT_PREOP_CALLBACK_STATUS
MiniFilterPreWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION nameInfo = NULL;
    
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    
    status = FltGetFileNameInformation(
        Data,
        FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
        &nameInfo
    );
    
    if (NT_SUCCESS(status)) {
        FltParseFileNameInformation(nameInfo);
        
        // Only log writes to target extensions
        if (IsTargetExtension(&nameInfo->Extension)) {
            DbgPrint("MiniFilter: PreWrite - Target file: %wZ\n", &nameInfo->Name);
        }
        
        FltReleaseFileNameInformation(nameInfo);
    }
    
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
```

### Example 6: Denying Operations

```c
// Deny write operations to read-only files
FLT_PREOP_CALLBACK_STATUS
MiniFilterPreWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    FILE_BASIC_INFORMATION fileInfo;
    NTSTATUS status;
    
    UNREFERENCED_PARAMETER(CompletionContext);
    
    // Query file attributes
    status = FltQueryInformationFile(
        FltObjects->Instance,
        FltObjects->FileObject,
        &fileInfo,
        sizeof(FILE_BASIC_INFORMATION),
        FileBasicInformation,
        NULL
    );
    
    if (NT_SUCCESS(status)) {
        // Check if file is read-only
        if (fileInfo.FileAttributes & FILE_ATTRIBUTE_READONLY) {
            DbgPrint("MiniFilter: Denying write to read-only file\n");
            
            // Deny the write operation
            Data->IoStatus.Status = STATUS_ACCESS_DENIED;
            Data->IoStatus.Information = 0;
            
            return FLT_PREOP_COMPLETE;
        }
    }
    
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
```

## Extending the Driver

### Example 7: Adding SetInformation Callback

```c
// In MiniFilter.c, add new callback prototype
FLT_PREOP_CALLBACK_STATUS
MiniFilterPreSetInformation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);

// Add to Callbacks array
CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      MiniFilterPreCreate,
      MiniFilterPostCreate },
      
    { IRP_MJ_SET_INFORMATION,
      0,
      MiniFilterPreSetInformation,
      NULL },
      
    // ... other callbacks ...
    
    { IRP_MJ_OPERATION_END }
};

// Implement the callback
FLT_PREOP_CALLBACK_STATUS
MiniFilterPreSetInformation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    FILE_INFORMATION_CLASS fileInfoClass;
    
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    
    fileInfoClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
    
    switch (fileInfoClass) {
        case FileDispositionInformation:
            DbgPrint("MiniFilter: File deletion requested\n");
            break;
            
        case FileRenameInformation:
            DbgPrint("MiniFilter: File rename requested\n");
            break;
            
        case FileAllocationInformation:
            DbgPrint("MiniFilter: File allocation change\n");
            break;
            
        default:
            DbgPrint("MiniFilter: SetInformation class: %d\n", fileInfoClass);
            break;
    }
    
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
```

### Example 8: Adding Context Support

```c
// Define context structure in MiniFilter.h
typedef struct _STREAM_CONTEXT {
    ULONG FileAccessCount;
    LARGE_INTEGER CreationTime;
    WCHAR FileName[260];
} STREAM_CONTEXT, *PSTREAM_CONTEXT;

// Register context type
CONST FLT_CONTEXT_REGISTRATION ContextRegistration[] = {
    { FLT_STREAM_CONTEXT,
      0,
      NULL,
      sizeof(STREAM_CONTEXT),
      'cSmF' },  // Tag for stream context
      
    { FLT_CONTEXT_END }
};

// Update FilterRegistration
CONST FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,
    ContextRegistration,  // Add context registration
    Callbacks,
    // ... rest of registration ...
};

// Use context in callbacks
FLT_PREOP_CALLBACK_STATUS
MiniFilterPreRead(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    NTSTATUS status;
    PSTREAM_CONTEXT streamCtx = NULL;
    
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(CompletionContext);
    
    // Get or create stream context
    status = FltGetStreamContext(
        FltObjects->Instance,
        FltObjects->FileObject,
        &streamCtx
    );
    
    if (status == STATUS_NOT_FOUND) {
        // Allocate new context
        status = FltAllocateContext(
            FltObjects->Filter,
            FLT_STREAM_CONTEXT,
            sizeof(STREAM_CONTEXT),
            PagedPool,
            &streamCtx
        );
        
        if (NT_SUCCESS(status)) {
            RtlZeroMemory(streamCtx, sizeof(STREAM_CONTEXT));
            KeQuerySystemTime(&streamCtx->CreationTime);
            
            status = FltSetStreamContext(
                FltObjects->Instance,
                FltObjects->FileObject,
                FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                streamCtx,
                NULL
            );
        }
    }
    
    if (NT_SUCCESS(status) && streamCtx != NULL) {
        // Update access count
        streamCtx->FileAccessCount++;
        
        DbgPrint("MiniFilter: File accessed %d times\n", 
                 streamCtx->FileAccessCount);
        
        FltReleaseContext(streamCtx);
    }
    
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
```

## Testing

### Example 9: Automated Test Suite

```powershell
# test_suite.ps1 - Comprehensive test suite

function Test-MiniFilter {
    param(
        [string]$TestDir = "C:\MiniFilterTest"
    )
    
    Write-Host "=== MiniFilter Test Suite ===" -ForegroundColor Cyan
    Write-Host ""
    
    # Test 1: Load driver
    Write-Host "Test 1: Loading driver..." -ForegroundColor Yellow
    fltmc load MiniFilter
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  PASS: Driver loaded" -ForegroundColor Green
    } else {
        Write-Host "  FAIL: Driver load failed" -ForegroundColor Red
        return
    }
    
    # Test 2: Verify driver is running
    Write-Host "Test 2: Verifying driver status..." -ForegroundColor Yellow
    $filterOutput = fltmc filters | Select-String "MiniFilter"
    if ($filterOutput) {
        Write-Host "  PASS: Driver is running" -ForegroundColor Green
    } else {
        Write-Host "  FAIL: Driver not found" -ForegroundColor Red
    }
    
    # Test 3: File creation
    Write-Host "Test 3: Testing file creation..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $TestDir -Force | Out-Null
    $testFile = Join-Path $TestDir "test.txt"
    "Test" | Out-File $testFile
    if (Test-Path $testFile) {
        Write-Host "  PASS: File created" -ForegroundColor Green
    } else {
        Write-Host "  FAIL: File creation failed" -ForegroundColor Red
    }
    
    # Test 4: File read
    Write-Host "Test 4: Testing file read..." -ForegroundColor Yellow
    $content = Get-Content $testFile
    if ($content -eq "Test") {
        Write-Host "  PASS: File read successful" -ForegroundColor Green
    } else {
        Write-Host "  FAIL: File read failed" -ForegroundColor Red
    }
    
    # Test 5: File write
    Write-Host "Test 5: Testing file write..." -ForegroundColor Yellow
    "Updated" | Out-File $testFile
    $content = Get-Content $testFile
    if ($content -eq "Updated") {
        Write-Host "  PASS: File write successful" -ForegroundColor Green
    } else {
        Write-Host "  FAIL: File write failed" -ForegroundColor Red
    }
    
    # Test 6: File deletion
    Write-Host "Test 6: Testing file deletion..." -ForegroundColor Yellow
    Remove-Item $testFile
    if (!(Test-Path $testFile)) {
        Write-Host "  PASS: File deleted" -ForegroundColor Green
    } else {
        Write-Host "  FAIL: File deletion failed" -ForegroundColor Red
    }
    
    # Cleanup
    Remove-Item $TestDir -Recurse -Force -ErrorAction SilentlyContinue
    
    Write-Host ""
    Write-Host "=== Test Suite Complete ===" -ForegroundColor Cyan
    Write-Host "Check DebugView for detailed filter logs" -ForegroundColor Yellow
}

# Run tests
Test-MiniFilter
```

### Example 10: Performance Test

```powershell
# performance_test.ps1 - Test filter performance impact

function Measure-FilterPerformance {
    param(
        [int]$FileCount = 1000,
        [string]$TestDir = "C:\MiniFilterPerfTest"
    )
    
    Write-Host "=== MiniFilter Performance Test ===" -ForegroundColor Cyan
    
    # Create test directory
    New-Item -ItemType Directory -Path $TestDir -Force | Out-Null
    
    # Test without filter
    Write-Host "`nUnloading filter for baseline test..." -ForegroundColor Yellow
    fltmc unload MiniFilter 2>$null
    
    $baselineTime = Measure-Command {
        for ($i = 0; $i -lt $FileCount; $i++) {
            "Test" | Out-File (Join-Path $TestDir "file_$i.txt")
        }
    }
    
    # Cleanup
    Remove-Item (Join-Path $TestDir "*") -Force
    
    # Test with filter
    Write-Host "Loading filter for comparison test..." -ForegroundColor Yellow
    fltmc load MiniFilter
    
    $filterTime = Measure-Command {
        for ($i = 0; $i -lt $FileCount; $i++) {
            "Test" | Out-File (Join-Path $TestDir "file_$i.txt")
        }
    }
    
    # Cleanup
    Remove-Item $TestDir -Recurse -Force
    
    # Results
    Write-Host "`n=== Results ===" -ForegroundColor Cyan
    Write-Host "File count: $FileCount"
    Write-Host "Baseline time: $($baselineTime.TotalSeconds) seconds"
    Write-Host "With filter time: $($filterTime.TotalSeconds) seconds"
    Write-Host "Overhead: $([math]::Round((($filterTime.TotalSeconds - $baselineTime.TotalSeconds) / $baselineTime.TotalSeconds) * 100, 2))%"
}

Measure-FilterPerformance
```

## Notes

- Always run scripts with administrator privileges
- Enable test signing mode for development drivers
- Use DebugView to monitor filter activity
- Test in a virtual machine before production use
- Check System event log for errors
