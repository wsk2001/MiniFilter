#include <fltKernel.h>
#include <dontuse.h>
#include "../Common/TssShield.h"

#define MAX_WATCH_DIRS 10
#define MAX_PATH_SIZE 260

PFLT_FILTER gFilterHandle = NULL;
PFLT_PORT gServerPort = NULL;
PFLT_PORT gClientPort = NULL;
UNICODE_STRING gWatchDirectories[MAX_WATCH_DIRS];
ULONG gNumWatchDirs = 0;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS TssShieldUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
FLT_PREOP_CALLBACK_STATUS TssShieldPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
NTSTATUS TssShieldPortConnect(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID* ConnectionPortCookie);
VOID TssShieldPortDisconnect(PVOID ConnectionCookie);
NTSTATUS TssShieldPortMessage(PVOID PortCookie, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength, PULONG ReturnOutputBufferLength);

const FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      TssShieldPreCreate,
      NULL
    },
    { IRP_MJ_OPERATION_END }
};

const FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,
    NULL,
    Callbacks,
    TssShieldUnload,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    DbgPrint("TssShield: DriverEntry\n");

    NTSTATUS status;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING uniString;
    PSECURITY_DESCRIPTOR sd;

    status = FltRegisterFilter(DriverObject, &FilterRegistration, &gFilterHandle);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);
    if (!NT_SUCCESS(status)) {
        FltUnregisterFilter(gFilterHandle);
        return status;
    }

    RtlInitUnicodeString(&uniString, TSS_SHIELD_PORT_NAME);
    InitializeObjectAttributes(&oa, &uniString, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, sd);

    status = FltCreateCommunicationPort(gFilterHandle, &gServerPort, &oa, NULL, TssShieldPortConnect, TssShieldPortDisconnect, TssShieldPortMessage, 1);
    FltFreeSecurityDescriptor(sd);

    if (!NT_SUCCESS(status)) {
        FltUnregisterFilter(gFilterHandle);
        return status;
    }

    status = FltStartFiltering(gFilterHandle);
    if (!NT_SUCCESS(status)) {
        FltCloseCommunicationPort(gServerPort);
        FltUnregisterFilter(gFilterHandle);
    }

    return status;
}

NTSTATUS TssShieldUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Flags);
    DbgPrint("TssShield: TssShieldUnload\n");
    FltCloseCommunicationPort(gServerPort);
    FltUnregisterFilter(gFilterHandle);

    for (ULONG i = 0; i < gNumWatchDirs; i++) {
        ExFreePool(gWatchDirectories[i].Buffer);
    }
    gNumWatchDirs = 0;

    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS TssShieldPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    NTSTATUS status;
    BOOLEAN isDirWatched = FALSE;

    if (gClientPort == NULL || gNumWatchDirs == 0) {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
    if (!NT_SUCCESS(status)) {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    for (ULONG i = 0; i < gNumWatchDirs; i++) {
        if (RtlPrefixUnicodeString(&gWatchDirectories[i], &FileNameInfo->Name, TRUE)) {
            isDirWatched = TRUE;
            break;
        }
    }

    if (!isDirWatched) {
        FltReleaseFileNameInformation(FileNameInfo);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    TSS_SHIELD_REQUEST request;
    request.ProcessId = FltGetRequestorProcessId(Data);
    wcsncpy(request.FilePath, FileNameInfo->Name.Buffer, MAX_PATH_SIZE - 1);
    request.FilePath[MAX_PATH_SIZE - 1] = L'\0';

    FltReleaseFileNameInformation(FileNameInfo);

    TSS_SHIELD_REPLY reply;
    ULONG replyLength = sizeof(reply);

    LARGE_INTEGER timeout;
    timeout.QuadPart = -10000000; // 1 second

    status = FltSendMessage(gFilterHandle, &gClientPort, &request, sizeof(request), &reply, &replyLength, &timeout);

    if (NT_SUCCESS(status) && replyLength == sizeof(reply) && !reply.Allow) {
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        return FLT_PREOP_COMPLETE;
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


NTSTATUS TssShieldPortConnect(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID* ConnectionPortCookie)
{
    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    UNREFERENCED_PARAMETER(ConnectionPortCookie);

    DbgPrint("TssShield: Agent connected.\n");
    gClientPort = ClientPort;
    return STATUS_SUCCESS;
}

VOID TssShieldPortDisconnect(PVOID ConnectionCookie)
{
    UNREFERENCED_PARAMETER(ConnectionCookie);
    DbgPrint("TssShield: Agent disconnected.\n");
    FltCloseClientPort(gFilterHandle, &gClientPort);
    gClientPort = NULL;
}

NTSTATUS TssShieldPortMessage(PVOID PortCookie, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength, PULONG ReturnOutputBufferLength)
{
    UNREFERENCED_PARAMETER(PortCookie);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ReturnOutputBufferLength);

    if (gNumWatchDirs < MAX_WATCH_DIRS && InputBuffer != NULL && InputBufferLength > 0)
    {
        gWatchDirectories[gNumWatchDirs].Buffer = (PWCH)ExAllocatePool2(POOL_FLAG_PAGED, InputBufferLength, 'wdct');
        if (gWatchDirectories[gNumWatchDirs].Buffer)
        {
            RtlCopyMemory(gWatchDirectories[gNumWatchDirs].Buffer, InputBuffer, InputBufferLength);
            gWatchDirectories[gNumWatchDirs].Length = (USHORT)InputBufferLength;
            gWatchDirectories[gNumWatchDirs].MaximumLength = (USHORT)InputBufferLength;
            gNumWatchDirs++;
            DbgPrint("TssShield: Added watch directory: %wZ\n", &gWatchDirectories[gNumWatchDirs - 1]);
        }
    }

    return STATUS_SUCCESS;
}
