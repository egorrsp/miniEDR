#pragma once

#include <fltKernel.h>

#include "telemetry.h"
#include "queue.h"

#define NT_DEVICE_NAME L"\\Device\\miniEDR"
#define DOS_DEVICE_NAME L"\\DosDevice\\miniEDR"

// Get event from queue
#define IOCTL_GET_EVENT CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x800, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS \
)

DRIVER_INITIALIZE DriverEntry;

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
_Dispatch_type_(IRP_MJ_CLEANUP)
DRIVER_DISPATCH MiniEdrCreateClose;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH MiniEdrDeviceControl;

DRIVER_UNLOAD UnloadDriver;

NTSTATUS InitMiniEDR(
	_In_ PDRIVER_OBJECT DriverObject
);

VOID UnloadMiniEDR(VOID);

extern PDRIVER_OBJECT g_DriverObject;
extern PDEVICE_OBJECT g_ControlDeviceObject;

