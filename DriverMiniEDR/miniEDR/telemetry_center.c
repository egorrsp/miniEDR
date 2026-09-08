#include "miniedr.h"
#include "network.h"

// Main initialization telemetry function
NTSTATUS InitTelemetry(
	PDEVICE_OBJECT DeviceObject
)
{
	NTSTATUS status = FALSE;
	BOOLEAN processWatcherInitialized = FALSE;
	BOOLEAN fileMappingWatcherInitialized = FALSE;

	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID,
		DPFLTR_ERROR_LEVEL,
		"miniedr: InitTelemetry start 0x%08X\n",
		status
	);

	status = ProcessWatcher();

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniedr: ProcessWatcher down: 0x%08X\n",
			status
		);

		return status;
	}
	processWatcherInitialized = TRUE;

	status = FileMappingWatcher();

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniedr: FileMappingWatcher down: 0x%08X\n",
			status
		);

		goto Cleanup;
	}
	fileMappingWatcherInitialized = TRUE;

	status = InitNetworkFilter(DeviceObject);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniedr: InitNetworkFilter down: 0x%08X\n",
			status
		);

		goto Cleanup;
	}

	return STATUS_SUCCESS;

Cleanup:
	if (fileMappingWatcherInitialized)
	{
		PsRemoveLoadImageNotifyRoutine(FileNotifyCallback);
	}

	if (processWatcherInitialized)
	{
		PsSetCreateProcessNotifyRoutineEx(
			ProcessNotifyCallback,
			TRUE
		);
	}

	return status;
}

VOID CloseTelemetry(VOID)
{
	NTSTATUS status;

	CloseNetworkFilter();

	status = PsSetCreateProcessNotifyRoutineEx(
		ProcessNotifyCallback,
		TRUE
	);

	if (!NT_SUCCESS(status)) 
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniEDR: failed to unregister process callback: 0x%08X\n",
			status
		);
	}

	status = PsRemoveLoadImageNotifyRoutine(
		FileNotifyCallback
	);

	if (!NT_SUCCESS(status)) 
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniEDR: failed to unregister image callback: 0x%08X\n",
			status
		);
	}

	return;
}
