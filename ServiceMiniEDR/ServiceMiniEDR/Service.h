#pragma once

#include <Windows.h>

#define DRIVER_NAME L"\\\\.\\miniEDR"

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

HANDLE OpenMiniEDRDriver();

DWORD WINAPI TelemetryWorker(
	LPVOID Parameter
);

// Get event from queue
#define IOCTL_GET_EVENT CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x800, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS \
)

VOID ClenupService();