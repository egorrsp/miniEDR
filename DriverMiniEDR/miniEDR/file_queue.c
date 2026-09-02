#include "miniedr.h"

BOOLEAN PushFileEvent(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	PFILE_EVENT event;

	event = ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(FILE_EVENT),
		'VErP'
	);

	if (event == NULL)
	{
		return FALSE;
	}

	RtlZeroMemory(
		event,
		sizeof(FILE_EVENT)
	);

	event->EventData.EventType = EVENT_FILE_MAPPED;
	event->EventData.ImageBase = ImageInfo->ImageBase;
	event->EventData.ImageSignatureLevel = ImageInfo->ImageSignatureLevel;
	event->EventData.ImageSignatureType = ImageInfo->ImageSignatureType;
	event->EventData.SystemModeImage = ImageInfo->SystemModeImage;
	event->EventData.ImageSize = ImageInfo->ImageSize;
	event->EventData.ProcessId = ProcessId;

	KeQuerySystemTimePrecise(
		&event->EventData.Timestamp
	);

	if (FullImageName != NULL) 
	{
		USHORT bytesToCopy = FullImageName->Length;
		if (bytesToCopy > sizeof(event->EventData.ImagePath) - sizeof(WCHAR))
		{
			bytesToCopy = sizeof(event->EventData.ImagePath) - sizeof(WCHAR);
		}
		
		RtlCopyMemory(
			event->EventData.ImagePath,
			FullImageName->Buffer,
			bytesToCopy
		);
		event->EventData.ImagePath[bytesToCopy / sizeof(WCHAR)] = L'\0';
	}

	return PushFileEventToQueue(event);
}

BOOLEAN PushFileEventToQueue(
	PFILE_EVENT event
)
{
	if (!PushEventToQueue(&event->ListEntry))
	{
		ExFreePool(event);
		return FALSE;
	}

	return TRUE;
}
