#include "GuiMiniEDR.h"

#include <stdio.h>

#define MAX_VISIBLE_EVENT_ROWS 10000

HWND g_EventList = NULL;

static VOID FormatEventTimestamp(
    LARGE_INTEGER Timestamp,
    LPWSTR Buffer,
    SIZE_T BufferCount
)
{
    FILETIME utcTime = { 0 };
    FILETIME localTime = { 0 };
    SYSTEMTIME systemTime = { 0 };

    utcTime.dwLowDateTime = Timestamp.LowPart;
    utcTime.dwHighDateTime = Timestamp.HighPart;

    if (Timestamp.QuadPart <= 0 ||
        !FileTimeToLocalFileTime(&utcTime, &localTime) ||
        !FileTimeToSystemTime(&localTime, &systemTime))
    {
        swprintf_s(Buffer, BufferCount, L"-");
        return;
    }

    swprintf_s(
        Buffer,
        BufferCount,
        L"%04u-%02u-%02u %02u:%02u:%02u.%03u",
        systemTime.wYear,
        systemTime.wMonth,
        systemTime.wDay,
        systemTime.wHour,
        systemTime.wMinute,
        systemTime.wSecond,
        systemTime.wMilliseconds
    );
}

VOID AddEventRow(
    LARGE_INTEGER Timestamp,
    LPCWSTR Type,
    LPCWSTR Pid,
    LPCWSTR Tid,
    LPCWSTR Object,
    LPCWSTR Details
)
{
    WCHAR time[32] = { 0 };

    if (g_EventList == NULL)
    {
        return;
    }

    int row = ListView_GetItemCount(g_EventList);

    if (row >= MAX_VISIBLE_EVENT_ROWS)
    {
        ListView_DeleteItem(g_EventList, 0);
        row--;
    }

    LVITEMW item = { 0 };

    FormatEventTimestamp(
        Timestamp,
        time,
        _countof(time)
    );

    item.mask = LVIF_TEXT;
    item.iItem = row;
    item.iSubItem = 0;
    item.pszText = time;

    row = ListView_InsertItem(
        g_EventList,
        &item
    );

    if (row == -1)
    {
        return;
    }

    ListView_SetItemText(
        g_EventList,
        row,
        1,
        (LPWSTR)(Type != NULL ? Type : L"")
    );

    ListView_SetItemText(
        g_EventList,
        row,
        2,
        (LPWSTR)(Pid != NULL ? Pid : L"")
    );

    ListView_SetItemText(
        g_EventList,
        row,
        3,
        (LPWSTR)(Tid != NULL ? Tid : L"")
    );

    ListView_SetItemText(
        g_EventList,
        row,
        4,
        (LPWSTR)(Object != NULL ? Object : L"")
    );

    ListView_SetItemText(
        g_EventList,
        row,
        5,
        (LPWSTR)(Details != NULL ? Details : L"")
    );

    ListView_EnsureVisible(
        g_EventList,
        row,
        FALSE
    );
}
