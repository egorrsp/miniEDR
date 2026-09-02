#include "miniEDR.h"
#include <wdmsec.h>
#include <initguid.h>

// {EDB09868-A485-47DB-B8F2-B9DCB985409C}
DEFINE_GUID(
	MINI_EDR_DEVICE_CLASS_GUID,
	0xedb09868,
	0xa485,
	0x47db,
	0xb8, 0xf2,
	0xb9, 0xdc, 0xb9, 0x85, 0x40, 0x9c
);

PDEVICE_OBJECT g_ControlDeviceObject = NULL;

NTSTATUS InitMiniEDR(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	NTSTATUS status = STATUS_SUCCESS;

	UNICODE_STRING ntUnicodeString;
	UNICODE_STRING ntWin32NameString;
	PDEVICE_OBJECT deviceObject = NULL;

	RtlInitUnicodeString(&ntUnicodeString, NT_DEVICE_NAME);
	RtlInitUnicodeString(&ntWin32NameString, DOS_DEVICE_NAME);

	status = IoCreateDeviceSecure(
		DriverObject,
		0,
		&ntUnicodeString,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&SDDL_DEVOBJ_SYS_ALL_ADM_ALL,
		&MINI_EDR_DEVICE_CLASS_GUID,
		&deviceObject
	);

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniEDR: IoCreateDeviceSecure failed: 0x%08X\n",
			status
		);
		return status;
	}

	status = IoCreateSymbolicLink(
		&ntWin32NameString, &ntUnicodeString
	);

	if (!NT_SUCCESS(status)) {
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID,
			DPFLTR_ERROR_LEVEL,
			"miniEDR: IoCreateSymbolicLink failed: 0x%08X\n",
			status
		);
		IoDeleteDevice(deviceObject);
		return status;
	}

	deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	g_ControlDeviceObject = deviceObject;

	return status;
}

_Use_decl_annotations_
NTSTATUS MiniEdrCreateClose(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

VOID UnloadMiniEDR(VOID)
{
	PDEVICE_OBJECT deviceObject = g_ControlDeviceObject;

	UNICODE_STRING ntWin32NameString;
	g_ControlDeviceObject = NULL;

	RtlInitUnicodeString(&ntWin32NameString, DOS_DEVICE_NAME);

	IoDeleteSymbolicLink(
		&ntWin32NameString
	);

	if (deviceObject != NULL) {
		IoDeleteDevice(
			deviceObject
		);
	}
}

_Use_decl_annotations_
NTSTATUS MiniEdrDeviceControl(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION irpSp;
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG_PTR information = 0;

	irpSp = IoGetCurrentIrpStackLocation(Irp);

	switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_GET_EVENT:
	{
		PLIST_ENTRY entry;
		PVOID eventData = NULL;
		ULONG eventDataSize = 0;
		ULONG eventType;
		ULONG outputBufferLength =
			irpSp->Parameters.DeviceIoControl.OutputBufferLength;

		entry = PopEventFromQueue();
		if (entry == NULL)
		{
			status = STATUS_NO_MORE_ENTRIES;
			break;
		}

		eventType = GetEventType(entry);
		switch (eventType)
		{
		case EVENT_PROCESS_CREATE:
		case EVENT_PROCESS_CLOSE:
			eventData = &CONTAINING_RECORD(
				entry, PROCESS_EVENT, ListEntry)->EventData;
			eventDataSize = sizeof(PROCESS_EVENT_DATA);
			break;

		case EVENT_FILE_MAPPED:
			eventData = &CONTAINING_RECORD(
				entry, FILE_EVENT, ListEntry)->EventData;
			eventDataSize = sizeof(FILE_EVENT_DATA);
			break;

		case EVENT_FILE_CREATE:
			eventData = &CONTAINING_RECORD(
				entry, FILE_CREATE_EVENT, ListEntry)->EventData;
			eventDataSize = sizeof(FILE_CREATE_EVENT_DATA);
			break;

		case EVENT_FILE_WRITE:
			eventData = &CONTAINING_RECORD(
				entry, FILE_WRITE_EVENT, ListEntry)->EventData;
			eventDataSize = sizeof(FILE_WRITE_EVENT_DATA);
			break;

		case EVENT_FILE_RENAME:
		case EVENT_FILE_DELETE:
		case EVENT_FILE_SET_INFORMATION:
			eventData = &CONTAINING_RECORD(
				entry, FILE_EVENT_SET_INFO, ListEntry)->EventData;
			eventDataSize = sizeof(FILE_EVENT_SET_INFO_DATA);
			break;

		case EVENT_NETWORK_CONNECT:
			eventData = &CONTAINING_RECORD(
				entry, NETWORK_EVENT, ListEntry)->EventData;
			eventDataSize = sizeof(NETWORK_EVENT_DATA);
			break;

		default:
			status = STATUS_DATA_ERROR;
			ExFreePool(entry);
			break;
		}

		if (eventData == NULL)
		{
			break;
		}

		if (outputBufferLength < eventDataSize)
		{
			if (!RequeueEventAtHead(entry))
			{
				ExFreePool(entry);
			}

			information = eventDataSize;
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		RtlCopyMemory(
			Irp->AssociatedIrp.SystemBuffer,
			eventData,
			eventDataSize
		);

		information = eventDataSize;
		status = STATUS_SUCCESS;
		ExFreePool(entry);
		break;
	}

	default:
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = information;

	IoCompleteRequest(
		Irp,
		IO_NO_INCREMENT
	);

	return status;
}
