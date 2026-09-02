#include "miniedr.h"

BOOLEAN PushFilterIOCreateEvent(
    PFLT_CALLBACK_DATA Data
)
{
    NTSTATUS status;
    PFILE_CREATE_EVENT event;
    PFLT_FILE_NAME_INFORMATION nameInfo = NULL;

    ULONG options =
        Data->Iopb->Parameters.Create.Options;

    status = FltGetFileNameInformation(
        Data,
        FLT_FILE_NAME_NORMALIZED |
        FLT_FILE_NAME_QUERY_DEFAULT,
        &nameInfo
    );

    if (!NT_SUCCESS(status) || nameInfo == NULL)
    {
        return FALSE;
    }

    event = ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(FILE_CREATE_EVENT),
        'VErP'
    );

    if (event == NULL)
    {
        FltReleaseFileNameInformation(nameInfo);
        return FALSE;
    }

    RtlZeroMemory(
        event,
        sizeof(FILE_CREATE_EVENT)
    );

    event->EventData.EventType = EVENT_FILE_CREATE;

    event->EventData.ProcessId =
        (HANDLE)(ULONG_PTR)FltGetRequestorProcessId(Data);

    event->EventData.ThreadId =
        Data->Thread != NULL ? PsGetThreadId(Data->Thread) : NULL;

    KeQuerySystemTimePrecise(
        &event->EventData.Timestamp
    );

    event->EventData.Status =
        Data->IoStatus.Status;

    event->EventData.DesiredAccess =
        Data->Iopb
        ->Parameters
        .Create
        .SecurityContext
        ->DesiredAccess;

    event->EventData.CreateDisposition =
        (options >> 24) & 0xFF;

    event->EventData.CreateOptions =
        options & 0x00FFFFFF;

    event->EventData.FileAttributes =
        Data->Iopb->Parameters.Create.FileAttributes;

    event->EventData.ShareAccess =
        Data->Iopb->Parameters.Create.ShareAccess;

    event->EventData.RequestorMode =
        Data->RequestorMode;

    ULONG bytesToCopy = nameInfo->Name.Length;

    if (bytesToCopy >
        sizeof(event->EventData.FilePath) - sizeof(WCHAR))
    {
        bytesToCopy =
            sizeof(event->EventData.FilePath) - sizeof(WCHAR);
    }

    RtlCopyMemory(
        event->EventData.FilePath,
        nameInfo->Name.Buffer,
        bytesToCopy
    );

    event->EventData.FilePath[
        bytesToCopy / sizeof(WCHAR)
    ] = L'\0';

    FltReleaseFileNameInformation(nameInfo);

    return PushFilterIOCreateEventToQueue(event);
}


BOOLEAN PushFilterIOCreateEventToQueue(
    PFILE_CREATE_EVENT event
)
{
	if (!PushEventToQueue(&event->ListEntry))
	{
		ExFreePool(event);
		return FALSE;
	}

	return TRUE;
}


BOOLEAN PushFilterIOWriteEvent(
    PFLT_CALLBACK_DATA Data
)
{
    NTSTATUS status;
    PFILE_WRITE_EVENT event;
    PFLT_FILE_NAME_INFORMATION nameInfo = NULL;

    status = FltGetFileNameInformation(
        Data,
        FLT_FILE_NAME_NORMALIZED |
        FLT_FILE_NAME_QUERY_DEFAULT,
        &nameInfo
    );

    if (!NT_SUCCESS(status) || nameInfo == NULL)
    {
        return FALSE;
    }

    event = ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(FILE_WRITE_EVENT),
        'VErP'
    );

    if (event == NULL)
    {
        FltReleaseFileNameInformation(nameInfo);
        return FALSE;
    }

    RtlZeroMemory(
        event,
        sizeof(FILE_WRITE_EVENT)
    );

    event->EventData.EventType = EVENT_FILE_WRITE;

    event->EventData.ProcessId =
        (HANDLE)(ULONG_PTR)FltGetRequestorProcessId(Data);

    event->EventData.ThreadId =
        Data->Thread != NULL ? PsGetThreadId(Data->Thread) : NULL;

    KeQuerySystemTimePrecise(
        &event->EventData.Timestamp
    );

    event->EventData.Status =
        Data->IoStatus.Status;

    event->EventData.RequestedLength =
        Data->Iopb->Parameters.Write.Length;

    event->EventData.BytesWritten =
        Data->IoStatus.Information;

    event->EventData.ByteOffset =
        Data->Iopb->Parameters.Write.ByteOffset;

    event->EventData.RequestorMode =
        Data->RequestorMode;

    ULONG bytesToCopy = nameInfo->Name.Length;

    if (bytesToCopy >
        sizeof(event->EventData.FilePath) - sizeof(WCHAR))
    {
        bytesToCopy =
            sizeof(event->EventData.FilePath) - sizeof(WCHAR);
    }

    RtlCopyMemory(
        event->EventData.FilePath,
        nameInfo->Name.Buffer,
        bytesToCopy
    );

    event->EventData.FilePath[
        bytesToCopy / sizeof(WCHAR)
    ] = L'\0';

    FltReleaseFileNameInformation(nameInfo);

    return PushFilterIOWriteEventToQueue(event);
}


BOOLEAN PushFilterIOWriteEventToQueue(
    PFILE_WRITE_EVENT event
)
{
	if (!PushEventToQueue(&event->ListEntry))
	{
		ExFreePool(event);
		return FALSE;
	}

	return TRUE;
}


BOOLEAN PushFilterIOSetInformationEvent(
    PFLT_CALLBACK_DATA Data,
    PFILE_SET_INFO_CONTEXT Context
)
{
    PFILE_EVENT_SET_INFO event;

    if (Context == NULL)
    {
        return FALSE;
    }

    event = ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(FILE_EVENT_SET_INFO),
        'VErP'
    );

    if (event == NULL)
    {
        return FALSE;
    }

    RtlZeroMemory(
        event,
        sizeof(FILE_EVENT_SET_INFO)
    );

    event->EventData.InformationClass =
        Context->InformationClass;

    event->EventData.EventType = Context->EventType;
    event->EventData.DeleteFile = Context->DeleteFile;
    event->EventData.ReplaceIfExists = Context->ReplaceIfExists;

    RtlCopyMemory(
        event->EventData.FilePath,
        Context->FilePath,
        sizeof(event->EventData.FilePath)
    );

    RtlCopyMemory(
        event->EventData.NewFilePath,
        Context->NewFilePath,
        sizeof(event->EventData.NewFilePath)
    );

    event->EventData.ProcessId =
        (HANDLE)(ULONG_PTR)FltGetRequestorProcessId(Data);

    event->EventData.ThreadId =
        Data->Thread != NULL ? PsGetThreadId(Data->Thread) : NULL;

    KeQuerySystemTimePrecise(
        &event->EventData.Timestamp
    );

    event->EventData.Status =
        Data->IoStatus.Status;

    event->EventData.RequestorMode =
        Data->RequestorMode;

    return PushFilterIOSetInformationEventToQueue(event);
}


BOOLEAN PushFilterIOSetInformationEventToQueue(
    PFILE_EVENT_SET_INFO event
)
{
	if (!PushEventToQueue(&event->ListEntry))
	{
		ExFreePool(event);
		return FALSE;
	}

	return TRUE;
}
