#include "miniedr.h"

// File mapping callback routine and helpers
NTSTATUS FileMappingWatcher(VOID)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = PsSetLoadImageNotifyRoutine(
		FileNotifyCallback
	);

	return status;
}

VOID FileNotifyCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	BOOLEAN status;

	status = PushFileEvent(
		FullImageName,
		ProcessId,
		ImageInfo
	);

	if (!status)
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_WARNING_LEVEL,
			"miniEDR: image event was dropped\n"
		);
	}
}
