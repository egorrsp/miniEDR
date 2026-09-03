#include "Service.h"

SERVICE_STATUS_HANDLE g_ServiceStatusHandle = NULL;
SERVICE_STATUS g_ServiceStatus = { 0 };
HANDLE g_StopEvent = NULL;
HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;
HANDLE g_DriverTelemetryThread = INVALID_HANDLE_VALUE;

void WINAPI ServiceMain(
    DWORD argc,
    LPWSTR* argv
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    g_ServiceStatusHandle =
        RegisterServiceCtrlHandlerExW(
            L"miniEDRService",
            ServiceControlHandler,
            NULL
        );

    if (g_ServiceStatusHandle == NULL)
    {
        return;
    }

    ZeroMemory(
        &g_ServiceStatus,
        sizeof(g_ServiceStatus)
    );

    g_ServiceStatus.dwServiceType =
        SERVICE_WIN32_OWN_PROCESS;

    g_ServiceStatus.dwCurrentState =
        SERVICE_START_PENDING;

    g_ServiceStatus.dwControlsAccepted = 0;

    SetServiceStatus(
        g_ServiceStatusHandle,
        &g_ServiceStatus
    );

    g_StopEvent = CreateEventW(
        NULL,
        TRUE,
        FALSE,
        NULL
    );

    if (g_StopEvent == NULL)
    {
        g_ServiceStatus.dwCurrentState =
            SERVICE_STOPPED;

        g_ServiceStatus.dwWin32ExitCode =
            GetLastError();

        SetServiceStatus(
            g_ServiceStatusHandle,
            &g_ServiceStatus
        );

        return;
    }

    
    g_DriverHandle = OpenMiniEDRDriver();

    if (g_DriverHandle == INVALID_HANDLE_VALUE)
    {
        g_ServiceStatus.dwCurrentState =
            SERVICE_STOPPED;

        g_ServiceStatus.dwWin32ExitCode =
            GetLastError();

        SetServiceStatus(
            &g_ServiceStatusHandle,
            &g_ServiceStatus
        );

        return;
    }

    g_DriverTelemetryThread = CreateThread(
        NULL,
        0,
        TelemetryWorker,
        NULL,
        0,
        NULL
    );

    g_ServiceStatus.dwControlsAccepted =
        SERVICE_ACCEPT_STOP;

    g_ServiceStatus.dwCurrentState =
        SERVICE_RUNNING;

    SetServiceStatus(
        g_ServiceStatusHandle,
        &g_ServiceStatus
    );

    WaitForSingleObject(
        g_StopEvent,
        INFINITE
    );

    ClenupService();

    CloseHandle(
        g_StopEvent
    );

    g_StopEvent = NULL;

    g_ServiceStatus.dwControlsAccepted = 0;

    g_ServiceStatus.dwCurrentState =
        SERVICE_STOPPED;

    SetServiceStatus(
        g_ServiceStatusHandle,
        &g_ServiceStatus
    );
}



VOID ClenupService()
{
    SetEvent(
        g_StopEvent
    );

    WaitForSingleObject(
        g_DriverTelemetryThread,
        0
    );

    CloseHandle(
        g_DriverTelemetryThread
    );

    CloseHandle(
        g_DriverHandle
    );
}