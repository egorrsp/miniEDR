#include "miniedr.h"

BOOLEAN PushProcessCreateEvent(
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	PPROCESS_EVENT event;

	event = ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(PROCESS_EVENT),
		'VErP'
	);
	
	if (event == NULL) 
	{
		return FALSE;
	}

	RtlZeroMemory(
		event,
		sizeof(PROCESS_EVENT)
	);

	event->EventData.EventType = EVENT_PROCESS_CREATE;
	event->EventData.ProcessId = ProcessId;
	event->EventData.ProcessParentId = CreateInfo->ParentProcessId;
	
	KeQuerySystemTimePrecise(
		&event->EventData.Timestamp
	);

	if (CreateInfo->ImageFileName != NULL) 
	{
		USHORT bytesToCopy = CreateInfo->ImageFileName->Length;

		if (bytesToCopy > sizeof(event->EventData.ImagePath) - sizeof(WCHAR)) {
			bytesToCopy = sizeof(event->EventData.ImagePath) - sizeof(WCHAR);
		}

		RtlCopyMemory(
			event->EventData.ImagePath,
			CreateInfo->ImageFileName->Buffer,
			bytesToCopy
		);

		event->EventData.ImagePath[bytesToCopy / sizeof(WCHAR)] = L'\0';
	}

	return PushProcessEventToQueue(event);
}

BOOLEAN PushProcessCloseEvent(
	HANDLE ProcessId
)
{
	PPROCESS_EVENT event;

	event = ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(PROCESS_EVENT),
		'VErP'
	);

	if (event == NULL) {
		return FALSE;
	}

	RtlZeroMemory(
		event,
		sizeof(PROCESS_EVENT)
	);

	event->EventData.EventType = EVENT_PROCESS_CLOSE;
	event->EventData.ProcessId = ProcessId;
	event->EventData.ProcessParentId = NULL;
	
	KeQuerySystemTimePrecise(
		&event->EventData.Timestamp
	);

	event->EventData.ImagePath[0] = L'\0';

	return PushProcessEventToQueue(event);
}

BOOLEAN PushProcessEventToQueue(
	PPROCESS_EVENT event
)
{
	if (!PushEventToQueue(&event->ListEntry))
	{
		ExFreePool(event);
		return FALSE;
	}

	return TRUE;
}
