#pragma once

#include <Windows.h>

#include "ConnectionTypes.h"
#include "List.h"

#define SERVICE_PIPE_NAME L"\\\\.\\pipe\\miniEDR"
#define PIPE_RECONNECT_INTERVAL_MS 500
#define MINI_EDR_MAX_PAYLOAD_SIZE 4096

#define WM_APP_PIPE_EVENT (WM_APP + 1)

typedef struct _GUI_PIPE_EVENT_MESSAGE
{
	PIPE_EVENT_HEADER Header;
	BYTE Payload[ANYSIZE_ARRAY];

} GUI_PIPE_EVENT_MESSAGE, *PGUI_PIPE_EVENT_MESSAGE;

extern HANDLE g_PipeHandle;
extern HANDLE g_StopEvent;
