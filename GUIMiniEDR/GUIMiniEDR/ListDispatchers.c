#include "GuiMiniEDR.h"

#include <stdio.h>

VOID DisplayProcessEvent(
    PEVENT_PROCESS_CREATE_CLOSE_PIPE Event,
    ULONG EventType
)
{
    WCHAR pid[32] = { 0 };
    WCHAR details[128] = { 0 };

    Event->ImagePath[_countof(Event->ImagePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        details,
        _countof(details),
        L"PPID=%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessParentId
    );

    AddEventRow(
        Event->Timestamp,
        EventType == EVENT_PROCESS_CREATE
        ? L"PROCESS_CREATE"
        : L"PROCESS_CLOSE",
        pid,
        L"-",
        Event->ImagePath,
        details
    );
}


VOID DisplayFileMappedEvent(
    PEVENT_FILE_MAPPED_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR details[256] = { 0 };

    Event->ImagePath[_countof(Event->ImagePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        details,
        _countof(details),
        L"Base=0x%p Size=%llu System=%lu SigLevel=%lu SigType=%lu",
        Event->ImageBase,
        (unsigned long long)Event->ImageSize,
        Event->SystemModeImage,
        Event->ImageSignatureLevel,
        Event->ImageSignatureType
    );

    AddEventRow(
        Event->Timestamp,
        L"FILE_MAPPED",
        pid,
        L"-",
        Event->ImagePath,
        details
    );
}


VOID DisplayFileCreateEvent(
    PEVENT_FILE_CREATE_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR tid[32] = { 0 };
    WCHAR details[256] = { 0 };

    Event->FilePath[_countof(Event->FilePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        tid,
        _countof(tid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ThreadId
    );

    swprintf_s(
        details,
        _countof(details),
        L"Status=0x%08X Access=0x%08X Disp=%lu Options=0x%08X Attr=0x%08X Share=0x%08X Mode=%lu",
        (ULONG)Event->Status,
        Event->DesiredAccess,
        Event->CreateDisposition,
        Event->CreateOptions,
        Event->FileAttributes,
        Event->ShareAccess,
        Event->RequestorMode
    );

    AddEventRow(
        Event->Timestamp,
        L"FILE_CREATE",
        pid,
        tid,
        Event->FilePath,
        details
    );
}


VOID DisplayFileWriteEvent(
    PEVENT_FILE_WRITE_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR tid[32] = { 0 };
    WCHAR details[256] = { 0 };

    Event->FilePath[_countof(Event->FilePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        tid,
        _countof(tid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ThreadId
    );

    swprintf_s(
        details,
        _countof(details),
        L"Status=0x%08X Requested=%lu Written=%llu Offset=%lld Mode=%lu",
        (ULONG)Event->Status,
        Event->RequestedLength,
        (unsigned long long)Event->BytesWritten,
        (long long)Event->ByteOffset.QuadPart,
        Event->RequestorMode
    );

    AddEventRow(
        Event->Timestamp,
        L"FILE_WRITE",
        pid,
        tid,
        Event->FilePath,
        details
    );
}


VOID DisplayFileRenameEvent(
    PEVENT_FILE_RENAME_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR tid[32] = { 0 };
    WCHAR details[512] = { 0 };

    Event->FilePath[_countof(Event->FilePath) - 1] = L'\0';
    Event->NewFilePath[_countof(Event->NewFilePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        tid,
        _countof(tid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ThreadId
    );

    swprintf_s(
        details,
        _countof(details),
        L"NewPath=%s Status=0x%08X InfoClass=%lu Replace=%u Mode=%lu",
        Event->NewFilePath,
        (ULONG)Event->Status,
        Event->InformationClass,
        Event->ReplaceIfExists,
        Event->RequestorMode
    );

    AddEventRow(
        Event->Timestamp,
        L"FILE_RENAME",
        pid,
        tid,
        Event->FilePath,
        details
    );
}


VOID DisplayFileDeleteEvent(
    PEVENT_FILE_DELETE_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR tid[32] = { 0 };
    WCHAR details[256] = { 0 };

    Event->FilePath[_countof(Event->FilePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        tid,
        _countof(tid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ThreadId
    );

    swprintf_s(
        details,
        _countof(details),
        L"Status=0x%08X InfoClass=%lu Delete=%u Mode=%lu",
        (ULONG)Event->Status,
        Event->InformationClass,
        Event->DeleteFile,
        Event->RequestorMode
    );

    AddEventRow(
        Event->Timestamp,
        L"FILE_DELETE",
        pid,
        tid,
        Event->FilePath,
        details
    );
}


VOID DisplayFileSetInformationEvent(
    PEVENT_FILE_SET_INFORMATION_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR tid[32] = { 0 };
    WCHAR details[256] = { 0 };

    Event->FilePath[_countof(Event->FilePath) - 1] = L'\0';

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        tid,
        _countof(tid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ThreadId
    );

    swprintf_s(
        details,
        _countof(details),
        L"Status=0x%08X InfoClass=%lu Mode=%lu",
        (ULONG)Event->Status,
        Event->InformationClass,
        Event->RequestorMode
    );

    AddEventRow(
        Event->Timestamp,
        L"FILE_SET_INFORMATION",
        pid,
        tid,
        Event->FilePath,
        details
    );
}


VOID DisplayNetworkConnectEvent(
    PEVENT_NETWORK_CONNECT_PIPE Event
)
{
    WCHAR pid[32] = { 0 };
    WCHAR target[128] = { 0 };
    WCHAR details[128] = { 0 };

    swprintf_s(
        pid,
        _countof(pid),
        L"%llu",
        (unsigned long long)(ULONG_PTR)Event->ProcessId
    );

    swprintf_s(
        target,
        _countof(target),
        L"%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u",
        (Event->LocalAddress >> 24) & 0xff,
        (Event->LocalAddress >> 16) & 0xff,
        (Event->LocalAddress >> 8) & 0xff,
        Event->LocalAddress & 0xff,
        Event->LocalPort,
        (Event->RemoteAddress >> 24) & 0xff,
        (Event->RemoteAddress >> 16) & 0xff,
        (Event->RemoteAddress >> 8) & 0xff,
        Event->RemoteAddress & 0xff,
        Event->RemotePort
    );

    swprintf_s(
        details,
        _countof(details),
        L"Protocol=%u",
        Event->Protocol
    );

    AddEventRow(
        Event->Timestamp,
        L"NETWORK_CONNECT",
        pid,
        L"-",
        target,
        details
    );
}

VOID DispatchEvent(
    PIPE_EVENT_HEADER* Header,
    BYTE* Payload
)
{
    switch (Header->EventType)
    {
    case EVENT_PROCESS_CREATE:
    case EVENT_PROCESS_CLOSE:
    {
        if (
            Header->Size !=
            sizeof(EVENT_PROCESS_CREATE_CLOSE_PIPE)
            )
        {
            return;
        }

        DisplayProcessEvent(
            (PEVENT_PROCESS_CREATE_CLOSE_PIPE)Payload,
            Header->EventType
        );

        break;
    }

    case EVENT_FILE_MAPPED:
    {
        if (
            Header->Size !=
            sizeof(EVENT_FILE_MAPPED_PIPE)
            )
        {
            return;
        }

        DisplayFileMappedEvent(
            (PEVENT_FILE_MAPPED_PIPE)Payload
        );

        break;
    }

    case EVENT_FILE_CREATE:
    {
        if (
            Header->Size !=
            sizeof(EVENT_FILE_CREATE_PIPE)
            )
        {
            return;
        }

        DisplayFileCreateEvent(
            (PEVENT_FILE_CREATE_PIPE)Payload
        );

        break;
    }

    case EVENT_FILE_WRITE:
    {
        if (
            Header->Size !=
            sizeof(EVENT_FILE_WRITE_PIPE)
            )
        {
            return;
        }

        DisplayFileWriteEvent(
            (PEVENT_FILE_WRITE_PIPE)Payload
        );

        break;
    }

    case EVENT_FILE_RENAME:
    {
        if (
            Header->Size !=
            sizeof(EVENT_FILE_RENAME_PIPE)
            )
        {
            return;
        }

        DisplayFileRenameEvent(
            (PEVENT_FILE_RENAME_PIPE)Payload
        );

        break;
    }

    case EVENT_FILE_DELETE:
    {
        if (
            Header->Size !=
            sizeof(EVENT_FILE_DELETE_PIPE)
            )
        {
            return;
        }

        DisplayFileDeleteEvent(
            (PEVENT_FILE_DELETE_PIPE)Payload
        );

        break;
    }

    case EVENT_FILE_SET_INFORMATION:
    {
        if (
            Header->Size !=
            sizeof(EVENT_FILE_SET_INFORMATION_PIPE)
            )
        {
            return;
        }

        DisplayFileSetInformationEvent(
            (PEVENT_FILE_SET_INFORMATION_PIPE)Payload
        );

        break;
    }

    case EVENT_NETWORK_CONNECT:
    {
        if (
            Header->Size !=
            sizeof(EVENT_NETWORK_CONNECT_PIPE)
            )
        {
            return;
        }

        DisplayNetworkConnectEvent(
            (PEVENT_NETWORK_CONNECT_PIPE)Payload
        );

        break;
    }

    default:
        break;
    }
}
