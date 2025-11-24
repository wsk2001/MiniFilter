#pragma once

#define TSS_SHIELD_PORT_NAME L"\\TssShieldPort"

typedef struct _TSS_SHIELD_REQUEST {
    HANDLE ProcessId;
    WCHAR FilePath[260];
} TSS_SHIELD_REQUEST, *PTSS_SHIELD_REQUEST;

typedef struct _TSS_SHIELD_REPLY {
    BOOLEAN Allow;
} TSS_SHIELD_REPLY, *PTSS_SHIELD_REPLY;
