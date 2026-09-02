#pragma once

#include "telemetry.h"

#define MINI_EDR_MAX_QUEUED_EVENTS 4096

// For queue & spin lock
// Initialized in main queue file
extern LIST_ENTRY g_EventQueue;
extern KSPIN_LOCK g_QueueLock;
extern ULONG g_EventCount;

typedef struct _FILE_SET_INFO_CONTEXT
{
	ULONG EventType;
	FILE_INFORMATION_CLASS InformationClass;
	BOOLEAN DeleteFile;
	BOOLEAN ReplaceIfExists;
	WCHAR FilePath[MINI_EDR_PATH_CAPACITY];
	WCHAR NewFilePath[MINI_EDR_PATH_CAPACITY];

} FILE_SET_INFO_CONTEXT, *PFILE_SET_INFO_CONTEXT;

VOID InitializeEventQueue(VOID);

BOOLEAN PushEventToQueue(
	PLIST_ENTRY Entry
);

BOOLEAN RequeueEventAtHead(
	PLIST_ENTRY Entry
);

BOOLEAN PushProcessCreateEvent(
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

BOOLEAN PushProcessCloseEvent(
	HANDLE ProcessId
);

BOOLEAN PushFileEvent(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
);

BOOLEAN PushProcessEventToQueue(
	PPROCESS_EVENT event
);

BOOLEAN PushFileEventToQueue(
	PFILE_EVENT event
);

C_ASSERT(
	FIELD_OFFSET(PROCESS_EVENT, EventData.EventType) ==
	FIELD_OFFSET(FILE_EVENT, EventData.EventType)
);

C_ASSERT(FIELD_OFFSET(PROCESS_EVENT, ListEntry) == 0);
C_ASSERT(FIELD_OFFSET(FILE_EVENT, ListEntry) == 0);
C_ASSERT(FIELD_OFFSET(FILE_CREATE_EVENT, ListEntry) == 0);
C_ASSERT(FIELD_OFFSET(FILE_WRITE_EVENT, ListEntry) == 0);
C_ASSERT(FIELD_OFFSET(FILE_EVENT_SET_INFO, ListEntry) == 0);
C_ASSERT(FIELD_OFFSET(NETWORK_EVENT, ListEntry) == 0);

C_ASSERT(
	FIELD_OFFSET(PROCESS_EVENT, EventData.EventType) ==
	FIELD_OFFSET(FILE_CREATE_EVENT, EventData.EventType)
);
C_ASSERT(
	FIELD_OFFSET(PROCESS_EVENT, EventData.EventType) ==
	FIELD_OFFSET(FILE_WRITE_EVENT, EventData.EventType)
);
C_ASSERT(
	FIELD_OFFSET(PROCESS_EVENT, EventData.EventType) ==
	FIELD_OFFSET(FILE_EVENT_SET_INFO, EventData.EventType)
);
C_ASSERT(
	FIELD_OFFSET(PROCESS_EVENT, EventData.EventType) ==
	FIELD_OFFSET(NETWORK_EVENT, EventData.EventType)
);

BOOLEAN PushFilterIOCreateEvent(
	PFLT_CALLBACK_DATA Data
);

BOOLEAN PushFilterIOWriteEvent(
	PFLT_CALLBACK_DATA Data
);

BOOLEAN PushFilterIOCreateEventToQueue(
	PFILE_CREATE_EVENT event
);

BOOLEAN PushFilterIOWriteEventToQueue(
	PFILE_WRITE_EVENT event
);

BOOLEAN PushFilterIOSetInformationEvent(
	PFLT_CALLBACK_DATA Data,
	PFILE_SET_INFO_CONTEXT Context
);

BOOLEAN PushFilterIOSetInformationEventToQueue(
	PFILE_EVENT_SET_INFO event
);

BOOLEAN PushNetworkEventToQueue(
	PNETWORK_EVENT event
);

ULONG GetEventType(
	PLIST_ENTRY entry
);

PLIST_ENTRY PopEventFromQueue(VOID);

VOID ClearEventQueue(VOID);
