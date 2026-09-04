#include "Service.h"

#include <sddl.h>

#define PIPE_WRITE_TIMEOUT_MS 1000

static HANDLE g_PipeHandle = INVALID_HANDLE_VALUE;
static SRWLOCK g_PipeLock = SRWLOCK_INIT;

HANDLE CreateTelemetryPipe()
{
	PSECURITY_DESCRIPTOR securityDescriptor = NULL;
	SECURITY_ATTRIBUTES securityAttributes = { 0 };

	if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
		L"D:P(A;;GA;;;SY)(A;;GA;;;BA)(A;;GR;;;AU)",
		SDDL_REVISION_1,
		&securityDescriptor,
		NULL
	))
	{
		return INVALID_HANDLE_VALUE;
	}

	securityAttributes.nLength = sizeof(securityAttributes);
	securityAttributes.lpSecurityDescriptor = securityDescriptor;

	HANDLE pipe = CreateNamedPipe(
		SERVICE_PIPE_NAME,
		PIPE_ACCESS_OUTBOUND |
		FILE_FLAG_FIRST_PIPE_INSTANCE |
		FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE |
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT |
		PIPE_REJECT_REMOTE_CLIENTS,
		1,
		4096,
		4096,
		0,
		&securityAttributes
	);

	LocalFree(securityDescriptor);
	return pipe;
}

static BOOL WaitForPipeClient(
	HANDLE Pipe
)
{
	OVERLAPPED overlapped = { 0 };
	DWORD transferred = 0;
	BOOL connected = FALSE;

	overlapped.hEvent = CreateEventW(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if (overlapped.hEvent == NULL)
	{
		return FALSE;
	}

	if (ConnectNamedPipe(Pipe, &overlapped))
	{
		connected = TRUE;
	}
	else
	{
		DWORD error = GetLastError();

		if (error == ERROR_PIPE_CONNECTED)
		{
			connected = TRUE;
		}
		else if (error == ERROR_IO_PENDING)
		{
			HANDLE waitHandles[] = {
				g_StopEvent,
				overlapped.hEvent
			};

			DWORD waitResult = WaitForMultipleObjects(
				ARRAYSIZE(waitHandles),
				waitHandles,
				FALSE,
				INFINITE
			);

			if (waitResult == WAIT_OBJECT_0 + 1)
			{
				connected = GetOverlappedResult(
					Pipe,
					&overlapped,
					&transferred,
					FALSE
				);
			}
			else
			{
				CancelIoEx(Pipe, &overlapped);
				GetOverlappedResult(
					Pipe,
					&overlapped,
					&transferred,
					TRUE
				);
			}
		}
	}

	CloseHandle(overlapped.hEvent);
	return connected;
}

DWORD WINAPI PipeWorker(
	LPVOID Parameter
)
{
	UNREFERENCED_PARAMETER(Parameter);

	while (
		WaitForSingleObject(
			g_StopEvent,
			0
		) != WAIT_OBJECT_0
	)
	{
		HANDLE pipe = CreateTelemetryPipe();

		if (pipe == INVALID_HANDLE_VALUE)
		{
			if (WaitForSingleObject(g_StopEvent, 1000) ==
				WAIT_OBJECT_0)
			{
				break;
			}

			continue;
		}

		if (!WaitForPipeClient(pipe))
		{
			CloseHandle(pipe);
			continue;
		}

		AcquireSRWLockExclusive(&g_PipeLock);
		g_PipeHandle = pipe;
		ReleaseSRWLockExclusive(&g_PipeLock);

		for (;;)
		{
			DWORD waitResult = WaitForSingleObject(
				g_StopEvent,
				100
			);

			if (waitResult != WAIT_TIMEOUT)
			{
				break;
			}

			AcquireSRWLockShared(&g_PipeLock);
			BOOLEAN active = g_PipeHandle == pipe;
			ReleaseSRWLockShared(&g_PipeLock);

			if (!active)
			{
				break;
			}
		}

		AcquireSRWLockExclusive(&g_PipeLock);

		if (g_PipeHandle == pipe)
		{
			g_PipeHandle = INVALID_HANDLE_VALUE;
		}

		DisconnectNamedPipe(pipe);
		CloseHandle(pipe);

		ReleaseSRWLockExclusive(&g_PipeLock);
	}

	return 0;
}

BOOLEAN SendEventToGui(
	PVOID Buffer,
	DWORD Size
)
{
	HANDLE sourcePipe;
	HANDLE pipe = INVALID_HANDLE_VALUE;
	HANDLE process = GetCurrentProcess();

	AcquireSRWLockShared(&g_PipeLock);
	sourcePipe = g_PipeHandle;

	if (sourcePipe != INVALID_HANDLE_VALUE)
	{
		DuplicateHandle(
			process,
			sourcePipe,
			process,
			&pipe,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS
		);
	}

	ReleaseSRWLockShared(&g_PipeLock);

	if (pipe == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	OVERLAPPED overlapped = { 0 };
	DWORD bytesWritten = 0;
	BOOL result = FALSE;

	overlapped.hEvent = CreateEventW(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if (overlapped.hEvent == NULL)
	{
		CloseHandle(pipe);
		return FALSE;
	}

	result = WriteFile(
		pipe,
		Buffer,
		Size,
		NULL,
		&overlapped
	);

	if (!result)
	{
		DWORD error = GetLastError();

		if (error == ERROR_IO_PENDING)
		{
			HANDLE waitHandles[] = {
				g_StopEvent,
				overlapped.hEvent
			};

			DWORD waitResult = WaitForMultipleObjects(
				ARRAYSIZE(waitHandles),
				waitHandles,
				FALSE,
				PIPE_WRITE_TIMEOUT_MS
			);

			if (waitResult == WAIT_OBJECT_0 + 1)
			{
				result = GetOverlappedResult(
					pipe,
					&overlapped,
					&bytesWritten,
					FALSE
				);
			}
			else
			{
				CancelIoEx(pipe, &overlapped);
				GetOverlappedResult(
					pipe,
					&overlapped,
					&bytesWritten,
					TRUE
				);
			}
		}
	}
	else
	{
		result = GetOverlappedResult(
			pipe,
			&overlapped,
			&bytesWritten,
			FALSE
		);
	}

	CloseHandle(overlapped.hEvent);
	CloseHandle(pipe);

	if (!result || bytesWritten != Size)
	{
		AcquireSRWLockExclusive(&g_PipeLock);

		if (g_PipeHandle == sourcePipe)
		{
			g_PipeHandle = INVALID_HANDLE_VALUE;
		}

		ReleaseSRWLockExclusive(&g_PipeLock);
		return FALSE;
	}

	return TRUE;
}
