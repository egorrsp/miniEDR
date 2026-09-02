#include "miniedr.h"

LIST_ENTRY g_EventQueue;
KSPIN_LOCK g_QueueLock;
ULONG g_EventCount;

VOID InitializeEventQueue(VOID)
{
	InitializeListHead(&g_EventQueue);
	KeInitializeSpinLock(&g_QueueLock);
	g_EventCount = 0;
}

BOOLEAN PushEventToQueue(
	PLIST_ENTRY Entry
)
{
	KIRQL oldIrql;
	BOOLEAN queued = FALSE;

	KeAcquireSpinLock(&g_QueueLock, &oldIrql);

	if (g_EventCount < MINI_EDR_MAX_QUEUED_EVENTS)
	{
		InsertTailList(&g_EventQueue, Entry);
		g_EventCount++;
		queued = TRUE;
	}

	KeReleaseSpinLock(&g_QueueLock, oldIrql);
	return queued;
}

BOOLEAN RequeueEventAtHead(
	PLIST_ENTRY Entry
)
{
	KIRQL oldIrql;
	BOOLEAN queued = FALSE;

	KeAcquireSpinLock(&g_QueueLock, &oldIrql);

	if (g_EventCount < MINI_EDR_MAX_QUEUED_EVENTS)
	{
		InsertHeadList(&g_EventQueue, Entry);
		g_EventCount++;
		queued = TRUE;
	}

	KeReleaseSpinLock(&g_QueueLock, oldIrql);
	return queued;
}

ULONG GetEventType(
	PLIST_ENTRY entry
) 
{
	return *(PULONG)(
		(PUCHAR)entry +
		FIELD_OFFSET(PROCESS_EVENT, EventData.EventType)
	);
}

PLIST_ENTRY PopEventFromQueue(VOID)
{
	KIRQL oldIrql;
	PLIST_ENTRY entry = NULL;

	KeAcquireSpinLock(
		&g_QueueLock,
		&oldIrql
	);

	if (!IsListEmpty(&g_EventQueue))
	{
		entry = RemoveHeadList(
			&g_EventQueue
		);
		g_EventCount--;
	}

	KeReleaseSpinLock(
		&g_QueueLock,
		oldIrql
	);

	return entry;
}

VOID ClearEventQueue(VOID)
{
	PLIST_ENTRY event = NULL;

	for (;;)
	{
		event = PopEventFromQueue();

		if (event == NULL) {
			break;
		}

		ExFreePool(
			event
		);
	}

	return;
}
