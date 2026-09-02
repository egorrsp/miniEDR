#include "miniedr.h"

PDRIVER_OBJECT g_DriverObject = NULL;
static volatile LONG g_UnloadStarted = 0;

NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(RegistryPath);

	g_DriverObject = DriverObject;
	g_UnloadStarted = 0;

	InitializeEventQueue();

	DriverObject->DriverUnload = UnloadDriver;
	DriverObject->MajorFunction[IRP_MJ_CREATE] =
		MiniEdrCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] =
		MiniEdrCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] =
		MiniEdrCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
		MiniEdrDeviceControl;

	status = InitTelemetry();
	if (!NT_SUCCESS(status))
	{
		ClearEventQueue();
		return status;
	}

	status = InitMiniEDR(DriverObject);
	if (!NT_SUCCESS(status))
	{
		CloseTelemetry();
		ClearEventQueue();
		return status;
	}

	status = InitFileFilter(DriverObject);
	if (!NT_SUCCESS(status))
	{
		UnloadMiniEDR();
		CloseTelemetry();
		ClearEventQueue();
		return status;
	}

	status = InitNetworkFilter(g_ControlDeviceObject);
	if (!NT_SUCCESS(status))
	{
		CloseFileFilter();
		UnloadMiniEDR();
		CloseTelemetry();
		ClearEventQueue();
		return status;
	}

	return STATUS_SUCCESS;
}

VOID UnloadDriver(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	UNREFERENCED_PARAMETER(DriverObject);

	if (InterlockedCompareExchange(&g_UnloadStarted, 1, 0) != 0)
	{
		return;
	}

	CloseFileFilter();
	CloseNetworkFilter();
	CloseTelemetry();
	ClearEventQueue();

	UnloadMiniEDR();
	g_DriverObject = NULL;
}
