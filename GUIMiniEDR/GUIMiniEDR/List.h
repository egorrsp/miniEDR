#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#include "ConnectionTypes.h"

#pragma comment(lib, "Comctl32.lib")

extern HWND g_EventList;

BOOL CreateEventList(
    HWND ParentWindow
);

VOID ResizeEventList(
    HWND ParentWindow
);

BOOLEAN OpenPipe(VOID);

VOID ClosePipe(VOID);

BOOLEAN WINAPI ReadHeader(
	PIPE_EVENT_HEADER* eventHeader
);

BOOLEAN WINAPI ReadPayload(
	DWORD PayloadSize,
	BYTE* Buffer
);

VOID AddEventRow(
    LARGE_INTEGER Timestamp,
    LPCWSTR Type,
    LPCWSTR Pid,
    LPCWSTR Tid,
    LPCWSTR Object,
    LPCWSTR Details
);

DWORD WINAPI PipeWorker(
    LPVOID Parameter
);

VOID DispatchEvent(
    PIPE_EVENT_HEADER* Header,
    BYTE* Payload
);
