#include "GuiMiniEDR.h"

HANDLE g_StopEvent = NULL;

static HANDLE g_PipeWorkerThread = NULL;

static VOID FreePendingPipeMessages(
    HWND hWnd
)
{
    MSG pendingMessage = { 0 };

    while (PeekMessageW(
        &pendingMessage,
        hWnd,
        WM_APP_PIPE_EVENT,
        WM_APP_PIPE_EVENT,
        PM_REMOVE
    ))
    {
        HeapFree(
            GetProcessHeap(),
            0,
            (PVOID)pendingMessage.lParam
        );
    }
}

static VOID StopPipeWorker(
    HWND hWnd
)
{
    if (g_StopEvent != NULL)
    {
        SetEvent(g_StopEvent);
    }

    if (g_PipeWorkerThread != NULL)
    {
        CancelSynchronousIo(g_PipeWorkerThread);
        WaitForSingleObject(g_PipeWorkerThread, INFINITE);
        CloseHandle(g_PipeWorkerThread);
        g_PipeWorkerThread = NULL;
    }

    FreePendingPipeMessages(hWnd);
}

LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_CREATE:
    {
        return CreateEventList(hWnd) ? 0 : -1;
    }

    case WM_SIZE:
        ResizeEventList(hWnd);
        return 0;

    case WM_APP_PIPE_EVENT:
    {
        PGUI_PIPE_EVENT_MESSAGE eventMessage =
            (PGUI_PIPE_EVENT_MESSAGE)lParam;

        if (eventMessage != NULL)
        {
            DispatchEvent(
                &eventMessage->Header,
                eventMessage->Payload
            );

            HeapFree(
                GetProcessHeap(),
                0,
                eventMessage
            );
        }

        return 0;
    }

    case WM_DESTROY:
        StopPipeWorker(hWnd);
        g_EventList = NULL;
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(
        hWnd,
        message,
        wParam,
        lParam
    );
}


int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    const wchar_t CLASS_NAME[] = L"MiniEDRWindowClass";

    INITCOMMONCONTROLSEX commonControls = { 0 };

    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_LISTVIEW_CLASSES;

    if (!InitCommonControlsEx(&commonControls))
    {
        return 1;
    }

    g_StopEvent = CreateEventW(
        NULL,
        TRUE,
        FALSE,
        NULL
    );

    if (g_StopEvent == NULL)
    {
        return 1;
    }

    WNDCLASSEXW wc = { 0 };

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc))
    {
        CloseHandle(g_StopEvent);
        g_StopEvent = NULL;
        return 1;
    }

    HWND hWnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"miniEDR",
        WS_OVERLAPPEDWINDOW,

        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1000,
        700,

        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL)
    {
        CloseHandle(g_StopEvent);
        g_StopEvent = NULL;
        return 1;
    }

    g_PipeWorkerThread = CreateThread(
        NULL,
        0,
        PipeWorker,
        hWnd,
        0,
        NULL
    );

    if (g_PipeWorkerThread == NULL)
    {
        DestroyWindow(hWnd);
        CloseHandle(g_StopEvent);
        g_StopEvent = NULL;
        return 1;
    }

    ShowWindow(
        hWnd,
        nCmdShow
    );

    UpdateWindow(
        hWnd
    );

    MSG msg;

    BOOL messageResult;

    while ((messageResult = GetMessageW(
        &msg,
        NULL,
        0,
        0
    )) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CloseHandle(g_StopEvent);
    g_StopEvent = NULL;

    return messageResult == -1 ? 1 : (int)msg.wParam;
}
