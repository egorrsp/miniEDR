#include "Service.h"

DWORD WINAPI TelemetryWorker(
	LPVOID Parameter
)
{
	UNREFERENCED_PARAMETER(Parameter);

	BYTE event[MAX_EVENT_LENGTH];
	DWORD bytesReturned;

	while (
		WaitForSingleObject(
			g_StopEvent,
			0
	) != WAIT_OBJECT_0)
	{
		BOOL result = DeviceIoControl(
			g_DriverHandle,
			IOCTL_GET_EVENT,
			NULL,
			0,
			event,
			sizeof(event),
			&bytesReturned,
			NULL
		);

		if (!result)
		{
			DWORD error = GetLastError();

			if (error == ERROR_NO_MORE_ITEMS)
			{
				Sleep(100);
				continue;
			}

			break;
		}

		ProcessEvent(
			event,
			bytesReturned
		);
	}

	return 0;
}
