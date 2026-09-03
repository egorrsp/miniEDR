void ProcessEvent(
    BYTE* EventBuffer,
    DWORD EventSize
)
{
    if (EventBuffer == NULL || EventSize == 0)
    {
        return;
    }

    PMINI_EDR_EVENT event =
        (PMINI_EDR_EVENT)EventBuffer;

    switch (event->EventType)
    {
    case EVENT_PROCESS_CREATE:
        OutputDebugStringW(
            L"[miniEDR] PROCESS_CREATE\n"
        );
        break;

    case EVENT_PROCESS_CLOSE:
        OutputDebugStringW(
            L"[miniEDR] PROCESS_CLOSE\n"
        );
        break;

    case EVENT_FILE_MAPPED:
        OutputDebugStringW(
            L"[miniEDR] FILE_MAPPED\n"
        );
        break;

    default:
        OutputDebugStringW(
            L"[miniEDR] UNKNOWN_EVENT\n"
        );
        break;
    }
}