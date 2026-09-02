#include "miniedr.h"

// Process create/close callback routine and helpers
NTSTATUS ProcessWatcher(VOID)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = PsSetCreateProcessNotifyRoutineEx(
		ProcessNotifyCallback,
		FALSE
	);

	return status;
}

VOID ProcessNotifyCallback(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	UNREFERENCED_PARAMETER(Process);

	BOOLEAN status;

	if (CreateInfo == NULL) {
		status = PushProcessCloseEvent(
			ProcessId
		);
	}
	else {
		status = PushProcessCreateEvent(
			ProcessId,
			CreateInfo
		);
	}

	if (!status) {
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_WARNING_LEVEL,
			"miniEDR: process event was dropped\n"
		);
	}

	return;
}
