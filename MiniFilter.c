/*++

Module Name:

    MiniFilter.c

Abstract:

    This is the main module of the filesystem minifilter driver.
    It demonstrates basic filtering capabilities for file system operations.

Environment:

    Kernel mode

--*/

#include "MiniFilter.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

//
//  Global data
//

MINIFILTER_DATA MiniFilterData;

//
//  Operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      MiniFilterPreCreate,
      MiniFilterPostCreate },

    { IRP_MJ_WRITE,
      0,
      MiniFilterPreWrite,
      MiniFilterPostWrite },

    { IRP_MJ_READ,
      0,
      MiniFilterPreRead,
      MiniFilterPostRead },

    { IRP_MJ_OPERATION_END }
};

//
//  Context registration
//

CONST FLT_CONTEXT_REGISTRATION ContextRegistration[] = {

    { FLT_STREAM_CONTEXT,
      0,
      NULL,
      sizeof(MINIFILTER_STREAM_CONTEXT),
      MINIFILTER_POOL_TAG },

    { FLT_CONTEXT_END }
};

//
//  Filter registration
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof(FLT_REGISTRATION),         //  Size
    FLT_REGISTRATION_VERSION,         //  Version
    0,                                //  Flags
    ContextRegistration,              //  Context
    Callbacks,                        //  Operation callbacks
    MiniFilterUnload,                 //  FilterUnload
    MiniFilterInstanceSetup,          //  InstanceSetup
    MiniFilterInstanceQueryTeardown,  //  InstanceQueryTeardown
    MiniFilterInstanceTeardownStart,  //  InstanceTeardownStart
    MiniFilterInstanceTeardownComplete, //  InstanceTeardownComplete
    NULL,                             //  GenerateFileName
    NULL,                             //  NormalizeNameComponent
    NULL                              //  NormalizeContextCleanup
};

/*************************************************************************
    Driver initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
/*++

Routine Description:

    This is the initialization routine for this minifilter driver.
    This registers the minifilter with the filter manager and initiates
    filtering.

Arguments:

    DriverObject - Pointer to driver object created by the system.

    RegistryPath - Pointer to the registry path for this driver.

Return Value:

    STATUS_SUCCESS if initialization succeeds, error status otherwise.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("MiniFilter: DriverEntry\n"));

    //
    //  Register with filter manager
    //

    status = FltRegisterFilter(DriverObject,
                               &FilterRegistration,
                               &MiniFilterData.Filter);

    if (!NT_SUCCESS(status)) {
        KdPrint(("MiniFilter: FltRegisterFilter failed, status 0x%x\n", status));
        return status;
    }

    //
    //  Start filtering I/O
    //

    status = FltStartFiltering(MiniFilterData.Filter);

    if (!NT_SUCCESS(status)) {
        KdPrint(("MiniFilter: FltStartFiltering failed, status 0x%x\n", status));
        FltUnregisterFilter(MiniFilterData.Filter);
        return status;
    }

    KdPrint(("MiniFilter: Driver loaded successfully\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
MiniFilterUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
)
/*++

Routine Description:

    This is the unload routine for this minifilter driver.
    This unregisters the minifilter and performs cleanup.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("MiniFilter: Unload\n"));

    //
    //  Unregister the minifilter
    //

    FltUnregisterFilter(MiniFilterData.Filter);

    return STATUS_SUCCESS;
}

/*************************************************************************
    Instance setup/teardown routines.
*************************************************************************/

NTSTATUS
MiniFilterInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
/*++

Routine Description:

    This routine is called whenever a new instance is created on a volume.
    This allows for instance-specific initialization.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    Flags - Flags describing the reason for this attach.

    VolumeDeviceType - Device type of the volume being attached.

    VolumeFilesystemType - File system type of the volume being attached.

Return Value:

    STATUS_SUCCESS to attach, or error status to prevent attachment.

--*/
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    KdPrint(("MiniFilter: Instance Setup\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
MiniFilterInstanceQueryTeardown(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
/*++

Routine Description:

    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    Flags - Indicating where this detach request came from.

Return Value:

    STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    return STATUS_SUCCESS;
}

VOID
MiniFilterInstanceTeardownStart(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
)
/*++

Routine Description:

    This routine is called at the start of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("MiniFilter: Instance Teardown Start\n"));
}

VOID
MiniFilterInstanceTeardownComplete(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
)
/*++

Routine Description:

    This routine is called at the end of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("MiniFilter: Instance Teardown Complete\n"));
}

/*************************************************************************
    Pre/Post operation callbacks.
*************************************************************************/

FLT_PREOP_CALLBACK_STATUS
MiniFilterPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
/*++

Routine Description:

    This routine is called before a create operation.

Arguments:

    Data - Pointer to the filter callback data.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    CompletionContext - Returns a context pointer to be passed to post-operation callback.

Return Value:

    FLT_PREOP_SUCCESS_WITH_CALLBACK - Continue with post-operation callback.
    FLT_PREOP_SUCCESS_NO_CALLBACK - Continue without post-operation callback.

--*/
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    KdPrint(("MiniFilter: Pre-Create\n"));

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
MiniFilterPostCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
/*++

Routine Description:

    This routine is called after a create operation.

Arguments:

    Data - Pointer to the filter callback data.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    CompletionContext - Context pointer passed from pre-operation callback.

    Flags - Operation-specific flags.

Return Value:

    FLT_POSTOP_FINISHED_PROCESSING - Continue normal processing.

--*/
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("MiniFilter: Post-Create\n"));

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS
MiniFilterPreWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
/*++

Routine Description:

    This routine is called before a write operation.

Arguments:

    Data - Pointer to the filter callback data.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    CompletionContext - Returns a context pointer to be passed to post-operation callback.

Return Value:

    FLT_PREOP_SUCCESS_WITH_CALLBACK - Continue with post-operation callback.
    FLT_PREOP_SUCCESS_NO_CALLBACK - Continue without post-operation callback.

--*/
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    KdPrint(("MiniFilter: Pre-Write\n"));

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
MiniFilterPostWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
/*++

Routine Description:

    This routine is called after a write operation.

Arguments:

    Data - Pointer to the filter callback data.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    CompletionContext - Context pointer passed from pre-operation callback.

    Flags - Operation-specific flags.

Return Value:

    FLT_POSTOP_FINISHED_PROCESSING - Continue normal processing.

--*/
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("MiniFilter: Post-Write\n"));

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS
MiniFilterPreRead(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
/*++

Routine Description:

    This routine is called before a read operation.

Arguments:

    Data - Pointer to the filter callback data.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    CompletionContext - Returns a context pointer to be passed to post-operation callback.

Return Value:

    FLT_PREOP_SUCCESS_WITH_CALLBACK - Continue with post-operation callback.
    FLT_PREOP_SUCCESS_NO_CALLBACK - Continue without post-operation callback.

--*/
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    KdPrint(("MiniFilter: Pre-Read\n"));

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
MiniFilterPostRead(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
/*++

Routine Description:

    This routine is called after a read operation.

Arguments:

    Data - Pointer to the filter callback data.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure.

    CompletionContext - Context pointer passed from pre-operation callback.

    Flags - Operation-specific flags.

Return Value:

    FLT_POSTOP_FINISHED_PROCESSING - Continue normal processing.

--*/
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("MiniFilter: Post-Read\n"));

    return FLT_POSTOP_FINISHED_PROCESSING;
}
