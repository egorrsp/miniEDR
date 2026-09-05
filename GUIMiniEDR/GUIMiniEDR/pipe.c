#include "GuiMiniEDR.h"

HANDLE g_PipeHandle = INVALID_HANDLE_VALUE;

static BOOLEAN ReadExact(
	PVOID Buffer,
	DWORD Size
)
{
	DWORD totalBytesRead = 0;

	while (totalBytesRead < Size)
	{
		DWORD bytesRead = 0;
		BOOL result = ReadFile(
			g_PipeHandle,
			(BYTE*)Buffer + totalBytesRead,
			Size - totalBytesRead,
			&bytesRead,
			NULL
		);

		if (!result && GetLastError() != ERROR_MORE_DATA)
		{
			return FALSE;
		}

		if (bytesRead == 0)
		{
			return FALSE;
		}

		totalBytesRead += bytesRead;
	}

	return TRUE;
}


// Open pipe
BOOLEAN OpenPipe(VOID)
{
	g_PipeHandle = CreateFileW(
		SERVICE_PIPE_NAME,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (g_PipeHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}


VOID ClosePipe(VOID)
{
	if (g_PipeHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_PipeHandle);
		g_PipeHandle = INVALID_HANDLE_VALUE;
	}
}


BOOLEAN WINAPI ReadHeader(
	PIPE_EVENT_HEADER * eventHeader
)
{
	if (g_PipeHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return ReadExact(
		eventHeader,
		sizeof(*eventHeader)
	);
}


BOOLEAN WINAPI ReadPayload(
	DWORD PayloadSize,
	BYTE * Buffer
)
{
	if (g_PipeHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return ReadExact(
		Buffer,
		PayloadSize
	);
}
