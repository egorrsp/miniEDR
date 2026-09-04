#pragma once

#include <Windows.h>

#include "EventTypesList.h"
#include "ConnectionTypes.h"

#define DRIVER_NAME L"\\\\.\\miniEDR"
#define SERVICE_PIPE_NAME L"\\\\.\\pipe\\miniEDR"

#define MAX_EVENT_LENGTH 1072

DWORD WINAPI ServiceControlHandler(
    DWORD control,
    DWORD eventType,
    LPVOID eventData,
    LPVOID context
);

extern SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
extern SERVICE_STATUS g_ServiceStatus;
extern HANDLE g_StopEvent;
extern HANDLE g_DriverHandle;

VOID WINAPI ServiceMain(
	DWORD argc,
	LPWSTR* argv
);

HANDLE OpenMiniEDRDriver();

DWORD WINAPI TelemetryWorker(
	LPVOID Parameter
);

VOID ProcessEvent(
	BYTE* EventBuffer,
	DWORD EventSize
);

HANDLE CreateTelemetryPipe();

DWORD WINAPI PipeWorker(
	LPVOID Parameter
);

BOOLEAN SendEventToGui(
	PVOID Buffer,
	DWORD Size
);

// Get event from queue
#define IOCTL_GET_EVENT CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x800, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS \
)

VOID CleanupService();
