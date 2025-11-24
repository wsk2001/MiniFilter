# MiniFilter Developer Guide

## Architecture Overview

The MiniFilter driver is a file system mini-filter that operates in the Windows kernel mode. It uses the Filter Manager framework to intercept and monitor file system I/O operations.

### Architecture Diagram

```
User Mode Applications
        |
        | (IRP)
        v
I/O Manager
        |
        v
Filter Manager (FltMgr.sys)
        |
        v
+-------------------+
| MiniFilter Driver |
| (MiniFilter.sys)  |
+-------------------+
        |
        v
File System Driver (NTFS, FAT32, etc.)
        |
        v
Storage Stack
```

## Key Components

### 1. Driver Entry Point

The `DriverEntry` function initializes the driver:
- Registers the filter with Filter Manager
- Registers callback routines
- Starts filtering operations

```c
NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);
```

### 2. Filter Registration Structure

Defines the filter's properties and capabilities:

```c
CONST FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,                          // Flags
    NULL,                       // Context
    Callbacks,                  // Operation callbacks
    MiniFilterUnload,           // Unload callback
    MiniFilterInstanceSetup,    // Instance setup
    // ... additional callbacks
};
```

### 3. Operation Callbacks

The driver registers callbacks for specific IRP operations:

#### IRP_MJ_CREATE
Called when a file is opened or created.
- **Pre-operation**: `MiniFilterPreCreate`
- **Post-operation**: `MiniFilterPostCreate`

#### IRP_MJ_READ
Called before read operations.
- **Pre-operation**: `MiniFilterPreRead`

#### IRP_MJ_WRITE
Called before write operations.
- **Pre-operation**: `MiniFilterPreWrite`

### 4. Instance Management

Manages filter instances on different volumes:
- **Setup**: `MiniFilterInstanceSetup`
- **Teardown Start**: `MiniFilterInstanceTeardownStart`
- **Teardown Complete**: `MiniFilterInstanceTeardownComplete`

## Callback Flow

### Pre-Operation Callbacks

Pre-operation callbacks are called before the operation is passed to the file system:

```
Application Request
    ↓
I/O Manager creates IRP
    ↓
Filter Manager intercepts
    ↓
MiniFilterPreXXX callback
    ↓
[Modify/Allow/Deny]
    ↓
Pass to file system OR Complete IRP
```

**Return Values:**
- `FLT_PREOP_SUCCESS_WITH_CALLBACK`: Continue and call post-operation
- `FLT_PREOP_SUCCESS_NO_CALLBACK`: Continue without post-operation
- `FLT_PREOP_COMPLETE`: Complete the IRP here
- `FLT_PREOP_PENDING`: Pend the operation

### Post-Operation Callbacks

Post-operation callbacks are called after the file system processes the operation:

```
File System completes IRP
    ↓
Filter Manager intercepts
    ↓
MiniFilterPostXXX callback
    ↓
[Examine/Modify results]
    ↓
Return to application
```

**Return Values:**
- `FLT_POSTOP_FINISHED_PROCESSING`: Processing complete
- `FLT_POSTOP_MORE_PROCESSING_REQUIRED`: Need more work

## Data Structures

### FLT_CALLBACK_DATA

Contains information about the I/O operation:

```c
typedef struct _FLT_CALLBACK_DATA {
    FLT_CALLBACK_DATA_FLAGS Flags;
    PETHREAD Thread;
    PFLT_IO_PARAMETER_BLOCK Iopb;
    IO_STATUS_BLOCK IoStatus;
    // ... more fields
} FLT_CALLBACK_DATA, *PFLT_CALLBACK_DATA;
```

### PCFLT_RELATED_OBJECTS

Provides handles to filter-related objects:

```c
typedef struct _FLT_RELATED_OBJECTS {
    USHORT Size;
    PFLT_FILTER Filter;
    PFLT_VOLUME Volume;
    PFLT_INSTANCE Instance;
    PFILE_OBJECT FileObject;
    PKTRANSACTION Transaction;
} FLT_RELATED_OBJECTS, *PFLT_RELATED_OBJECTS;
```

## Common Operations

### Getting File Name

```c
NTSTATUS status;
PFLT_FILE_NAME_INFORMATION nameInfo;

status = FltGetFileNameInformation(
    Data,
    FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
    &nameInfo
);

if (NT_SUCCESS(status)) {
    status = FltParseFileNameInformation(nameInfo);
    if (NT_SUCCESS(status)) {
        // Use nameInfo->Name, nameInfo->Volume, etc.
        DbgPrint("File: %wZ\n", &nameInfo->Name);
    }
    FltReleaseFileNameInformation(nameInfo);
}
```

### Allocating Context

```c
NTSTATUS status;
PSTREAM_CONTEXT streamContext;

status = FltAllocateContext(
    FltObjects->Filter,
    FLT_STREAM_CONTEXT,
    sizeof(STREAM_CONTEXT),
    PagedPool,
    &streamContext
);

if (NT_SUCCESS(status)) {
    // Initialize context
    status = FltSetStreamContext(
        FltObjects->Instance,
        FltObjects->FileObject,
        FLT_SET_CONTEXT_KEEP_IF_EXISTS,
        streamContext,
        NULL
    );
    FltReleaseContext(streamContext);
}
```

### Modifying I/O Parameters

```c
FLT_PREOP_CALLBACK_STATUS PreWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    // Deny writes to read-only files
    if (IsFileReadOnly(FltObjects->FileObject)) {
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;
        return FLT_PREOP_COMPLETE;
    }
    
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
```

## Debugging Techniques

### Debug Print Levels

```c
// Information
DbgPrint("MiniFilter: Information message\n");

// Warning
DbgPrint("MiniFilter: WARNING - potential issue\n");

// Error
DbgPrint("MiniFilter: ERROR - operation failed with status 0x%x\n", status);
```

### Assertions

```c
ASSERT(Data != NULL);
ASSERT(FltObjects != NULL);
ASSERT(NT_SUCCESS(status));
```

### WinDbg Commands

```
!fltkd.filters              # List all filters
!fltkd.filter MiniFilter    # Show MiniFilter details
!fltkd.instances            # Show all instances
!fltkd.volumes              # Show all volumes
!analyze -v                 # Analyze crash dump
```

## Best Practices

### 1. Performance

- **Minimize work in callbacks**: Keep callbacks fast
- **Use appropriate synchronization**: Avoid contention
- **Avoid blocking operations**: Use asynchronous patterns
- **Cache frequently accessed data**: Reduce overhead

### 2. Reliability

- **Handle all error cases**: Check return values
- **Use try-except blocks**: Protect against exceptions
- **Clean up resources**: Always release allocated resources
- **Validate input**: Never trust input data

### 3. Security

- **Validate pointers**: Check for NULL before dereferencing
- **Use SAL annotations**: Help static analysis tools
- **Avoid buffer overflows**: Use safe string functions
- **Implement least privilege**: Request minimal access

### 4. Code Quality

- **Follow coding standards**: Consistent style
- **Comment complex logic**: Explain "why", not "what"
- **Use meaningful names**: Clear and descriptive
- **Keep functions focused**: Single responsibility

## Testing

### Unit Testing

```c
// Test file create interception
VOID TestFileCreate() {
    // Create a test file
    // Verify pre-create callback was called
    // Verify post-create callback was called
    // Verify file was created successfully
}
```

### Integration Testing

1. Load the driver
2. Perform file operations:
   - Create files
   - Read files
   - Write files
   - Delete files
3. Monitor debug output
4. Verify expected behavior

### Stress Testing

```powershell
# PowerShell script for stress testing
for ($i = 0; $i -lt 10000; $i++) {
    $content = "Test content $i"
    $content | Out-File "test_$i.txt"
    Get-Content "test_$i.txt"
    Remove-Item "test_$i.txt"
}
```

## Common Pitfalls

### 1. Memory Leaks
- **Problem**: Forgetting to release contexts or allocated memory
- **Solution**: Always pair allocations with releases

### 2. Deadlocks
- **Problem**: Improper lock ordering or recursive locking
- **Solution**: Use consistent lock ordering, document locking strategy

### 3. IRQL Issues
- **Problem**: Calling pageable code at DISPATCH_LEVEL
- **Solution**: Check IRQL before operations, use appropriate APIs

### 4. Reference Counting
- **Problem**: Incorrect reference counting on objects
- **Solution**: Match every add-ref with a release

## Extending the Driver

### Adding New Callbacks

1. Define the callback function
2. Add to the `Callbacks` array
3. Implement the callback logic
4. Update documentation

### Adding Context Support

1. Define context structure
2. Register context types in `FilterRegistration`
3. Allocate and set contexts in callbacks
4. Release contexts properly

### Adding Communication Channel

1. Create communication port
2. Handle connection requests
3. Process messages from user mode
4. Send notifications to user mode

## Resources

- **WDK Documentation**: Comprehensive driver development docs
- **MSDN Forums**: Community support
- **OSR Online**: Expert driver development forum
- **Windows Internals**: Deep dive into Windows architecture
- **GitHub Samples**: Official Microsoft driver samples

## Contributing

When contributing to this project:
1. Follow the coding standards
2. Add appropriate comments
3. Update documentation
4. Test thoroughly
5. Submit pull requests with clear descriptions
