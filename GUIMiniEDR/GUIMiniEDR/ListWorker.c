#include "GuiMiniEDR.h"

DWORD WINAPI PipeWorker(
    LPVOID Parameter
)
{
    HWND targetWindow = (HWND)Parameter;

    while (WaitForSingleObject(g_StopEvent, 0) == WAIT_TIMEOUT)
    {
        if (!OpenPipe())
        {
            if (WaitForSingleObject(
                g_StopEvent,
                PIPE_RECONNECT_INTERVAL_MS
            ) != WAIT_TIMEOUT)
            {
                break;
            }

            continue;
        }

        while (WaitForSingleObject(g_StopEvent, 0) == WAIT_TIMEOUT)
        {
            PIPE_EVENT_HEADER header = { 0 };

            if (!ReadHeader(&header))
            {
                break;
            }

            if (header.Size == 0 ||
                header.Size > MINI_EDR_MAX_PAYLOAD_SIZE)
            {
                break;
            }

            SIZE_T allocationSize =
                FIELD_OFFSET(GUI_PIPE_EVENT_MESSAGE, Payload) +
                header.Size;

            PGUI_PIPE_EVENT_MESSAGE message = HeapAlloc(
                GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                allocationSize
            );

            if (message == NULL)
            {
                break;
            }

            message->Header = header;

            if (!ReadPayload(
                header.Size,
                message->Payload
            ))
            {
                HeapFree(
                    GetProcessHeap(),
                    0,
                    message
                );

                break;
            }

            if (!PostMessageW(
                targetWindow,
                WM_APP_PIPE_EVENT,
                0,
                (LPARAM)message
            ))
            {
                HeapFree(
                    GetProcessHeap(),
                    0,
                    message
                );

                break;
            }
        }

        ClosePipe();

        if (WaitForSingleObject(
            g_StopEvent,
            PIPE_RECONNECT_INTERVAL_MS
        ) != WAIT_TIMEOUT)
        {
            break;
        }
    }

    ClosePipe();
    return 0;
}
