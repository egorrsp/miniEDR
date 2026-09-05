#include "GuiMiniEDR.h"

#define EVENT_LIST_MARGIN 8

typedef struct _EVENT_LIST_COLUMN
{
    LPCWSTR Title;
    INT Width;

} EVENT_LIST_COLUMN;

BOOL CreateEventList(
    HWND ParentWindow
)
{
    static const EVENT_LIST_COLUMN columns[] = {
        { L"Time", 175 },
        { L"Event", 175 },
        { L"PID", 80 },
        { L"TID", 80 },
        { L"Object", 360 },
        { L"Details", 520 }
    };

    g_EventList = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD |
        WS_VISIBLE |
        WS_TABSTOP |
        LVS_REPORT |
        LVS_SINGLESEL |
        LVS_SHOWSELALWAYS,
        0,
        0,
        0,
        0,
        ParentWindow,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

    if (g_EventList == NULL)
    {
        return FALSE;
    }

    ListView_SetExtendedListViewStyleEx(
        g_EventList,
        0,
        LVS_EX_DOUBLEBUFFER |
        LVS_EX_FULLROWSELECT |
        LVS_EX_GRIDLINES
    );

    for (INT index = 0; index < ARRAYSIZE(columns); index++)
    {
        LVCOLUMNW column = { 0 };

        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = (LPWSTR)columns[index].Title;
        column.cx = columns[index].Width;
        column.iSubItem = index;

        if (ListView_InsertColumn(g_EventList, index, &column) == -1)
        {
            DestroyWindow(g_EventList);
            g_EventList = NULL;
            return FALSE;
        }
    }

    ResizeEventList(ParentWindow);
    return TRUE;
}

VOID ResizeEventList(
    HWND ParentWindow
)
{
    RECT clientRectangle = { 0 };

    if (g_EventList == NULL ||
        !GetClientRect(ParentWindow, &clientRectangle))
    {
        return;
    }

    SetWindowPos(
        g_EventList,
        NULL,
        EVENT_LIST_MARGIN,
        EVENT_LIST_MARGIN,
        max(0, clientRectangle.right - (EVENT_LIST_MARGIN * 2)),
        max(0, clientRectangle.bottom - (EVENT_LIST_MARGIN * 2)),
        SWP_NOACTIVATE | SWP_NOZORDER
    );
}
