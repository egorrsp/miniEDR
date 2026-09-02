#include "miniedr.h"

// Main initialization telemetry function
NTSTATUS InitTelemetry(VOID)
{
	NTSTATUS status;

	status = ProcessWatcher();

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = FileMappingWatcher();

	if (!NT_SUCCESS(status))
	{
		PsSetCreateProcessNotifyRoutineEx(
			ProcessNotifyCallback,
			TRUE
		);
		return status;
	}

	return status;
}

VOID CloseTelemetry(VOID)
{
	NTSTATUS status;

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
