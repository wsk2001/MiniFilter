# MiniFilter API Reference

## Driver Functions

### DriverEntry

```c
NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);
```

**Description:** Entry point for the minifilter driver. Called by the system when the driver is loaded.

**Parameters:**
- `DriverObject`: Pointer to the driver object
- `RegistryPath`: Registry path where driver parameters are stored

**Returns:** STATUS_SUCCESS on success, error code on failure

**Notes:** 
- Registers the filter with Filter Manager
- Initializes global data structures
- Starts filtering operations

---

### MiniFilterUnload

```c
NTSTATUS MiniFilterUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
);
```

**Description:** Called when the filter is being unloaded.

**Parameters:**
- `Flags`: Indicates if unload is mandatory

**Returns:** STATUS_SUCCESS

**Notes:**
- Unregisters the filter
- Cleans up resources
- Can deny unload if FLTFL_FILTER_UNLOAD_MANDATORY not set

---

### MiniFilterInstanceSetup

```c
NTSTATUS MiniFilterInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);
```

**Description:** Called when a new instance is created on a volume.

**Parameters:**
- `FltObjects`: Filter-related objects
- `Flags`: Reason for attach request
- `VolumeDeviceType`: Type of volume device
- `VolumeFilesystemType`: File system type

**Returns:** 
- STATUS_SUCCESS to attach
- STATUS_FLT_DO_NOT_ATTACH to skip volume

---

## Callback Functions

### MiniFilterPreCreate

```c
FLT_PREOP_CALLBACK_STATUS MiniFilterPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);
```

**Description:** Pre-operation callback for IRP_MJ_CREATE operations.

**Parameters:**
- `Data`: Callback data for the I/O operation
- `FltObjects`: Related objects
- `CompletionContext`: Context to pass to post-operation

**Returns:**
- `FLT_PREOP_SUCCESS_WITH_CALLBACK`: Request post-operation callback
- `FLT_PREOP_SUCCESS_NO_CALLBACK`: No post-operation callback
- `FLT_PREOP_COMPLETE`: Complete the operation here

**Called When:** Before a file is created or opened

---

### MiniFilterPostCreate

```c
FLT_POSTOP_CALLBACK_STATUS MiniFilterPostCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);
```

**Description:** Post-operation callback for IRP_MJ_CREATE operations.

**Parameters:**
- `Data`: Callback data for the I/O operation
- `FltObjects`: Related objects
- `CompletionContext`: Context from pre-operation
- `Flags`: Processing flags

**Returns:**
- `FLT_POSTOP_FINISHED_PROCESSING`: Processing complete

**Called When:** After a file is created or opened

---

### MiniFilterPreRead

```c
FLT_PREOP_CALLBACK_STATUS MiniFilterPreRead(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);
```

**Description:** Pre-operation callback for IRP_MJ_READ operations.

**Parameters:**
- `Data`: Callback data for the I/O operation
- `FltObjects`: Related objects
- `CompletionContext`: Context to pass to post-operation

**Returns:**
- `FLT_PREOP_SUCCESS_NO_CALLBACK`: No post-operation callback

**Called When:** Before a read operation on a file

---

### MiniFilterPreWrite

```c
FLT_PREOP_CALLBACK_STATUS MiniFilterPreWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);
```

**Description:** Pre-operation callback for IRP_MJ_WRITE operations.

**Parameters:**
- `Data`: Callback data for the I/O operation
- `FltObjects`: Related objects
- `CompletionContext`: Context to pass to post-operation

**Returns:**
- `FLT_PREOP_SUCCESS_NO_CALLBACK`: No post-operation callback

**Called When:** Before a write operation on a file

---

## Data Structures

### FLT_CALLBACK_DATA

Contains information about an I/O operation.

**Key Fields:**
- `Iopb`: I/O parameter block
- `IoStatus`: Status of the I/O operation
- `Thread`: Thread that originated the I/O
- `RequestorMode`: Kernel or user mode

### PCFLT_RELATED_OBJECTS

Contains handles to filter-related objects.

**Key Fields:**
- `Filter`: Handle to the filter
- `Volume`: Handle to the volume
- `Instance`: Handle to the instance
- `FileObject`: Handle to the file object

### FLT_REGISTRATION

Defines filter registration information.

**Key Fields:**
- `Size`: Structure size
- `Version`: Registration version
- `Flags`: Filter flags
- `ContextRegistration`: Context registration
- `OperationRegistration`: Operation callbacks

---

## Constants

### Return Values

**Pre-Operation:**
- `FLT_PREOP_SUCCESS_WITH_CALLBACK (0x00)`
- `FLT_PREOP_SUCCESS_NO_CALLBACK (0x01)`
- `FLT_PREOP_PENDING (0x02)`
- `FLT_PREOP_COMPLETE (0x03)`

**Post-Operation:**
- `FLT_POSTOP_FINISHED_PROCESSING (0x00)`
- `FLT_POSTOP_MORE_PROCESSING_REQUIRED (0x01)`

### Altitude

Activity Monitor Range: 360000-389999
Current Altitude: 370020

---

## Usage Examples

### Getting File Name

```c
PFLT_FILE_NAME_INFORMATION nameInfo;
NTSTATUS status;

status = FltGetFileNameInformation(
    Data,
    FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
    &nameInfo
);

if (NT_SUCCESS(status)) {
    FltParseFileNameInformation(nameInfo);
    DbgPrint("File: %wZ\n", &nameInfo->Name);
    FltReleaseFileNameInformation(nameInfo);
}
```

### Denying an Operation

```c
FLT_PREOP_CALLBACK_STATUS PreWrite(...) {
    // Deny write operation
    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
    Data->IoStatus.Information = 0;
    return FLT_PREOP_COMPLETE;
}
```

### Modifying Return Data

```c
FLT_POSTOP_CALLBACK_STATUS PostRead(...) {
    if (NT_SUCCESS(Data->IoStatus.Status)) {
        // Modify read data
        PVOID buffer = Data->Iopb->Parameters.Read.ReadBuffer;
        ULONG length = Data->IoStatus.Information;
        // ... modify buffer ...
    }
    return FLT_POSTOP_FINISHED_PROCESSING;
}
```

---

## Error Codes

Common NTSTATUS values:
- `STATUS_SUCCESS (0x00000000)`: Success
- `STATUS_ACCESS_DENIED (0xC0000022)`: Access denied
- `STATUS_INSUFFICIENT_RESOURCES (0xC000009A)`: Out of resources
- `STATUS_INVALID_PARAMETER (0xC000000D)`: Invalid parameter
- `STATUS_FLT_DO_NOT_ATTACH (0xC01C0001)`: Do not attach to volume

---

## Filter Manager APIs

Common FltMgr functions used:
- `FltRegisterFilter`: Register the filter
- `FltStartFiltering`: Start filtering
- `FltUnregisterFilter`: Unregister the filter
- `FltGetFileNameInformation`: Get file name
- `FltParseFileNameInformation`: Parse file name
- `FltReleaseFileNameInformation`: Release file name info
- `FltAllocateContext`: Allocate context
- `FltSetContext`: Set context
- `FltGetContext`: Get context
- `FltReleaseContext`: Release context reference
