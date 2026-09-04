#include "Service.h"

SERVICE_STATUS_HANDLE g_ServiceStatusHandle = NULL;
SERVICE_STATUS g_ServiceStatus = { 0 };
HANDLE g_StopEvent = NULL;
HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;
HANDLE g_DriverTelemetryThread = NULL;
HANDLE g_ServerPipeThread = NULL;

void WINAPI ServiceMain(
    DWORD argc,
    LPWSTR* argv
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
	DWORD startError = ERROR_SUCCESS;

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
		startError = GetLastError();
		goto StartupFailed;
    }

    g_DriverTelemetryThread = CreateThread(
        NULL,
        0,
        TelemetryWorker,
        NULL,
        0,
        NULL
    );

	if (g_DriverTelemetryThread == NULL)
	{
		startError = GetLastError();
		goto StartupFailed;
	}

    g_ServerPipeThread = CreateThread(
        NULL,
        0,
        PipeWorker,
        NULL,
        0,
        NULL
    );

	if (g_ServerPipeThread == NULL)
	{
		startError = GetLastError();
		goto StartupFailed;
	}

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

    CleanupService();

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

	return;

StartupFailed:
	CleanupService();

	if (g_StopEvent != NULL)
	{
		CloseHandle(g_StopEvent);
		g_StopEvent = NULL;
	}

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = startError;

	SetServiceStatus(
		g_ServiceStatusHandle,
		&g_ServiceStatus
	);
}



VOID CleanupService()
{
	if (g_StopEvent != NULL)
	{
		SetEvent(g_StopEvent);
	}

	if (g_DriverTelemetryThread != NULL)
	{
		CancelSynchronousIo(g_DriverTelemetryThread);
		WaitForSingleObject(g_DriverTelemetryThread, INFINITE);
		CloseHandle(g_DriverTelemetryThread);
		g_DriverTelemetryThread = NULL;
	}

	if (g_ServerPipeThread != NULL)
	{
		CancelSynchronousIo(g_ServerPipeThread);
		WaitForSingleObject(g_ServerPipeThread, INFINITE);
		CloseHandle(g_ServerPipeThread);
		g_ServerPipeThread = NULL;
	}

	if (g_DriverHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_DriverHandle);
		g_DriverHandle = INVALID_HANDLE_VALUE;
	}
}
