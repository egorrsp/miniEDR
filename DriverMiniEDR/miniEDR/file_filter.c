#include "miniedr.h"

PFLT_FILTER g_Filter;

static FLT_PREOP_CALLBACK_STATUS PreCreate(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID* CompletionContext
);

static FLT_PREOP_CALLBACK_STATUS PreWrite(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID* CompletionContext
);

static FLT_PREOP_CALLBACK_STATUS PreSetInformation(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID* CompletionContext
);

static FLT_POSTOP_CALLBACK_STATUS PostOperation(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID CompletionContext,
	FLT_POST_OPERATION_FLAGS Flags
);

static FLT_POSTOP_CALLBACK_STATUS PostOperationWhenSafe(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID CompletionContext,
	FLT_POST_OPERATION_FLAGS Flags
);

static NTSTATUS FilterUnloadCallback(
	FLT_FILTER_UNLOAD_FLAGS Flags
);

static VOID CaptureSetInformationContext(
	PFLT_CALLBACK_DATA Data,
	PFILE_SET_INFO_CONTEXT Context
);

static VOID CaptureFileName(
	PFLT_CALLBACK_DATA Data,
	WCHAR FilePath[MINI_EDR_PATH_CAPACITY]
);

static CONST FLT_OPERATION_REGISTRATION callbacks[] =
{
	{
		IRP_MJ_CREATE,
		0,
		PreCreate,
		PostOperation
	},

	{
		IRP_MJ_WRITE,
		0,
		PreWrite,
		PostOperation
	},

	{
		IRP_MJ_SET_INFORMATION,
		0,
		PreSetInformation,
		PostOperation
	},

	{ IRP_MJ_OPERATION_END }
};

static CONST FLT_REGISTRATION FilterRegistration =
{
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	callbacks,
	FilterUnloadCallback,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static NTSTATUS FilterUnloadCallback(
	FLT_FILTER_UNLOAD_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Flags);

	UnloadDriver(g_DriverObject);
	return STATUS_SUCCESS;
}

NTSTATUS InitFileFilter(
	PDRIVER_OBJECT DriverObject
)
{
	NTSTATUS status;

	status = FltRegisterFilter(
		DriverObject,
		&FilterRegistration,
		&g_Filter
	);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = FltStartFiltering(
		g_Filter
	);

	if (!NT_SUCCESS(status))
	{
		FltUnregisterFilter(g_Filter);
		g_Filter = NULL;
	}

	return status;
}

VOID CloseFileFilter(VOID)
{
	PFLT_FILTER filter = g_Filter;

	if (filter != NULL)
	{
		g_Filter = NULL;
		FltUnregisterFilter(filter);
	}
}

/// PRE interceptors is just fillers

static FLT_PREOP_CALLBACK_STATUS PreCreate(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID* CompletionContext
)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);

	*CompletionContext = NULL;

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

static FLT_PREOP_CALLBACK_STATUS PreWrite(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID* CompletionContext
)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);

	*CompletionContext = NULL;

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

static FLT_PREOP_CALLBACK_STATUS PreSetInformation(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID* CompletionContext
)
{
	UNREFERENCED_PARAMETER(FltObjects);

	PFILE_SET_INFO_CONTEXT context = ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(FILE_SET_INFO_CONTEXT),
		'cISE'
	);

	if (context == NULL)
	{
		*CompletionContext = NULL;
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	RtlZeroMemory(context, sizeof(FILE_SET_INFO_CONTEXT));
	CaptureFileName(Data, context->FilePath);
	CaptureSetInformationContext(Data, context);
	*CompletionContext = context;

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

static VOID CaptureFileName(
	PFLT_CALLBACK_DATA Data,
	WCHAR FilePath[MINI_EDR_PATH_CAPACITY]
)
{
	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION nameInfo = NULL;

	status = FltGetFileNameInformation(
		Data,
		FLT_FILE_NAME_NORMALIZED |
		FLT_FILE_NAME_QUERY_DEFAULT,
		&nameInfo
	);

	if (!NT_SUCCESS(status) || nameInfo == NULL)
	{
		return;
	}

	ULONG bytesToCopy = nameInfo->Name.Length;

	if (bytesToCopy >
		(MINI_EDR_PATH_CAPACITY * sizeof(WCHAR)) - sizeof(WCHAR))
	{
		bytesToCopy =
			(MINI_EDR_PATH_CAPACITY * sizeof(WCHAR)) - sizeof(WCHAR);
	}

	bytesToCopy &= ~(sizeof(WCHAR) - 1);

	RtlCopyMemory(FilePath, nameInfo->Name.Buffer, bytesToCopy);
	FilePath[bytesToCopy / sizeof(WCHAR)] = L'\0';

	FltReleaseFileNameInformation(nameInfo);
}

static VOID CaptureSetInformationContext(
	PFLT_CALLBACK_DATA Data,
	PFILE_SET_INFO_CONTEXT Context
)
{
	PVOID infoBuffer =
		Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
	ULONG infoLength =
		Data->Iopb->Parameters.SetFileInformation.Length;

	Context->EventType = EVENT_FILE_SET_INFORMATION;
	Context->InformationClass =
		Data->Iopb->Parameters.SetFileInformation.FileInformationClass;

	if (infoBuffer == NULL)
	{
		return;
	}

	__try
	{
		if (Data->RequestorMode != KernelMode &&
			!FLT_IS_SYSTEM_BUFFER(Data))
		{
			ProbeForRead(infoBuffer, infoLength, 1);
		}

		switch (Context->InformationClass)
		{
		case FileRenameInformation:
		case FileRenameInformationBypassAccessCheck:
		case FileRenameInformationEx:
		case FileRenameInformationExBypassAccessCheck:
		{
			PFILE_RENAME_INFORMATION info =
				(PFILE_RENAME_INFORMATION)infoBuffer;
			ULONG headerSize =
				FIELD_OFFSET(FILE_RENAME_INFORMATION, FileName);

			Context->EventType = EVENT_FILE_RENAME;

			if (infoLength < headerSize)
			{
				break;
			}

			if (Context->InformationClass == FileRenameInformation ||
				Context->InformationClass ==
					FileRenameInformationBypassAccessCheck)
			{
				Context->ReplaceIfExists = info->ReplaceIfExists;
			}
			else
			{
				Context->ReplaceIfExists =
					(info->Flags & FILE_RENAME_REPLACE_IF_EXISTS) != 0;
			}

			ULONG bytesToCopy = info->FileNameLength;
			ULONG availableBytes = infoLength - headerSize;

			if (bytesToCopy > availableBytes)
			{
				bytesToCopy = availableBytes;
			}

			if (bytesToCopy >
				sizeof(Context->NewFilePath) - sizeof(WCHAR))
			{
				bytesToCopy =
					sizeof(Context->NewFilePath) - sizeof(WCHAR);
			}

			bytesToCopy &= ~(sizeof(WCHAR) - 1);

			RtlCopyMemory(
				Context->NewFilePath,
				info->FileName,
				bytesToCopy
			);

			Context->NewFilePath[
				bytesToCopy / sizeof(WCHAR)
			] = L'\0';
			break;
		}

		case FileDispositionInformation:
		{
			if (infoLength >= sizeof(FILE_DISPOSITION_INFORMATION))
			{
				Context->DeleteFile =
					((PFILE_DISPOSITION_INFORMATION)infoBuffer)->DeleteFile;
			}

			if (Context->DeleteFile)
			{
				Context->EventType = EVENT_FILE_DELETE;
			}
			break;
		}

		case FileDispositionInformationEx:
		{
			if (infoLength >= sizeof(FILE_DISPOSITION_INFORMATION_EX))
			{
				Context->DeleteFile =
					(((PFILE_DISPOSITION_INFORMATION_EX)infoBuffer)->Flags &
						FILE_DISPOSITION_DELETE) != 0;
			}

			if (Context->DeleteFile)
			{
				Context->EventType = EVENT_FILE_DELETE;
			}
			break;
		}

		default:
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Context->EventType = EVENT_FILE_SET_INFORMATION;
		Context->DeleteFile = FALSE;
		Context->ReplaceIfExists = FALSE;
		Context->NewFilePath[0] = L'\0';
	}
}


/// POST interceptors

static FLT_POSTOP_CALLBACK_STATUS PostOperation(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID CompletionContext,
	FLT_POST_OPERATION_FLAGS Flags
)
{
	FLT_POSTOP_CALLBACK_STATUS postStatus =
		FLT_POSTOP_FINISHED_PROCESSING;

	if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
	{
		if (CompletionContext != NULL)
		{
			ExFreePool(CompletionContext);
		}

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if (!FltDoCompletionProcessingWhenSafe(
		Data,
		FltObjects,
		CompletionContext,
		Flags,
		PostOperationWhenSafe,
		&postStatus
	))
	{
		if (CompletionContext != NULL)
		{
			ExFreePool(CompletionContext);
		}

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	return postStatus;
}

static FLT_POSTOP_CALLBACK_STATUS PostOperationWhenSafe(
	PFLT_CALLBACK_DATA Data,
	PCFLT_RELATED_OBJECTS FltObjects,
	PVOID CompletionContext,
	FLT_POST_OPERATION_FLAGS Flags
)
{
	BOOLEAN queued = TRUE;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	if (!NT_SUCCESS(Data->IoStatus.Status))
	{
		if (CompletionContext != NULL)
		{
			ExFreePool(CompletionContext);
		}

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	switch (Data->Iopb->MajorFunction)
	{
	case IRP_MJ_CREATE:
		queued = PushFilterIOCreateEvent(Data);
		break;

	case IRP_MJ_WRITE:
		queued = PushFilterIOWriteEvent(Data);
		break;

	case IRP_MJ_SET_INFORMATION:
		queued = PushFilterIOSetInformationEvent(
			Data,
			(PFILE_SET_INFO_CONTEXT)CompletionContext
		);
		break;

	default:
		break;
	}

	if (CompletionContext != NULL)
	{
		ExFreePool(CompletionContext);
	}

	if (!queued)
	{
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_WARNING_LEVEL,
			"miniEDR: file event was dropped\n"
		);
	}

	return FLT_POSTOP_FINISHED_PROCESSING;
}
