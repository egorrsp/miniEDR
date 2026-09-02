#pragma once

#define EVENT_NETWORK_CONNECT ((ULONG)9)

typedef struct _NETWORK_EVENT_DATA
{
	ULONG EventType;

	HANDLE ProcessId;

	LARGE_INTEGER Timestamp;

	UINT8 Protocol;

	UINT32 LocalAddress;
	UINT32 RemoteAddress;

	UINT16 LocalPort;
	UINT16 RemotePort;

} NETWORK_EVENT_DATA, *PNETWORK_EVENT_DATA;

typedef struct _NETWORK_EVENT
{
	LIST_ENTRY ListEntry;

	NETWORK_EVENT_DATA EventData;

} NETWORK_EVENT, *PNETWORK_EVENT;
