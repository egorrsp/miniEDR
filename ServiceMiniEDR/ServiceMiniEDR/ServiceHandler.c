#include "Service.h"

DWORD WINAPI ServiceControlHandler(
    DWORD control,
    DWORD eventType,
    LPVOID eventData,
    LPVOID context
)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:

        g_ServiceStatus.dwCurrentState =
            SERVICE_STOP_PENDING;

        SetServiceStatus(
            g_ServiceStatusHandle,
            &g_ServiceStatus
        );

        SetEvent(
            g_StopEvent
        );

        break;
    }

    return NO_ERROR;
}