#include <Windows.h>

SERVICE_STATUS g_ServiceStatus;
SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
HANDLE g_StopEvent;

void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
DWORD WINAPI ServiceControlHandler(
    DWORD control,
    DWORD eventType,
    LPVOID eventData,
    LPVOID context
);

int wmain(void)
{
    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        { L"miniEDRService", ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcherW(serviceTable);

    return 0;
}