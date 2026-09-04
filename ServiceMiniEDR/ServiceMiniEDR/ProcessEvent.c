#include "Service.h"

#include <string.h>

VOID ProcessEvent(
	BYTE* event,
	DWORD eventSize
)
{
	if (event == NULL || eventSize < sizeof(ULONG))
	{
		return;
	}

	ULONG eventType = *(ULONG*)event;

	switch (eventType)
	{

	case EVENT_PROCESS_CLOSE:
	case EVENT_PROCESS_CREATE:
	{
		if (eventSize < sizeof(PROCESS_EVENT_DATA))
		{
			return;
		}

		PPROCESS_EVENT_DATA data = { 0 };

		data = (PPROCESS_EVENT_DATA)event;

		EVENT_PROCESS_CREATE_CLOSE_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.ProcessParentId =
			data->ProcessParentId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		memcpy(
			telemetry_pack_data.ImagePath,
			data->ImagePath,
			sizeof(telemetry_pack_data.ImagePath)
		);

		FINAL_STRUCT_PROCESS_EVENT final_data = { 0 };

		final_data.EventType =
			eventType;

		final_data.Size =
			sizeof(EVENT_PROCESS_CREATE_CLOSE_PIPE);

		final_data.Data =
			telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_FILE_MAPPED:
	{
		if (eventSize < sizeof(FILE_EVENT_DATA))
		{
			return;
		}

		PFILE_EVENT_DATA data = { 0 };

		data = (PFILE_EVENT_DATA)event;

		EVENT_FILE_MAPPED_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.ImageBase =
			data->ImageBase;

		telemetry_pack_data.ImageSize =
			data->ImageSize;

		telemetry_pack_data.SystemModeImage =
			data->SystemModeImage;

		telemetry_pack_data.ImageSignatureLevel =
			data->ImageSignatureLevel;

		telemetry_pack_data.ImageSignatureType =
			data->ImageSignatureType;

		memcpy(
			telemetry_pack_data.ImagePath,
			data->ImagePath,
			sizeof(telemetry_pack_data.ImagePath)
		);

		FINAL_STRUCT_FILE_MAPPED_EVENT final_data = { 0 };

		final_data.EventType =
			eventType;

		final_data.Size =
			sizeof(EVENT_FILE_MAPPED_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_FILE_CREATE:
	{
		if (eventSize < sizeof(FILE_CREATE_EVENT_DATA))
		{
			return;
		}

		PFILE_CREATE_EVENT_DATA data = { 0 };

		data = (PFILE_CREATE_EVENT_DATA)event;

		EVENT_FILE_CREATE_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.ThreadId =
			data->ThreadId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.Status =
			data->Status;

		telemetry_pack_data.DesiredAccess =
			data->DesiredAccess;

		telemetry_pack_data.CreateDisposition =
			data->CreateDisposition;

		telemetry_pack_data.CreateOptions =
			data->CreateOptions;

		telemetry_pack_data.FileAttributes =
			data->FileAttributes;

		telemetry_pack_data.ShareAccess =
			data->ShareAccess;

		telemetry_pack_data.RequestorMode =
			data->RequestorMode;

		memcpy(
			telemetry_pack_data.FilePath,
			data->FilePath,
			sizeof(telemetry_pack_data.FilePath)
		);

		FINAL_STRUCT_FILE_CREATE_EVENT final_data = { 0 };

		final_data.EventType = eventType;

		final_data.Size = sizeof(EVENT_FILE_CREATE_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_FILE_WRITE:
	{
		if (eventSize < sizeof(FILE_WRITE_EVENT_DATA))
		{
			return;
		}

		PFILE_WRITE_EVENT_DATA data =
			(PFILE_WRITE_EVENT_DATA)event;

		EVENT_FILE_WRITE_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.ThreadId =
			data->ThreadId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.Status =
			data->Status;

		telemetry_pack_data.RequestedLength =
			data->RequestedLength;

		telemetry_pack_data.BytesWritten =
			data->BytesWritten;

		telemetry_pack_data.ByteOffset =
			data->ByteOffset;

		telemetry_pack_data.RequestorMode =
			data->RequestorMode;

		memcpy(
			telemetry_pack_data.FilePath,
			data->FilePath,
			sizeof(telemetry_pack_data.FilePath)
		);

		FINAL_STRUCT_FILE_WRITE_EVENT final_data = { 0 };

		final_data.EventType = eventType;

		final_data.Size = sizeof(EVENT_FILE_WRITE_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_FILE_RENAME:
	{
		if (eventSize < sizeof(FILE_EVENT_SET_INFO_DATA))
		{
			return;
		}

		PFILE_EVENT_SET_INFO_DATA data =
			(PFILE_EVENT_SET_INFO_DATA)event;

		EVENT_FILE_RENAME_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.ThreadId =
			data->ThreadId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.Status =
			data->Status;

		telemetry_pack_data.InformationClass =
			data->InformationClass;

		telemetry_pack_data.RequestorMode =
			data->RequestorMode;

		telemetry_pack_data.ReplaceIfExists =
			data->ReplaceIfExists;

		memcpy(
			telemetry_pack_data.FilePath,
			data->FilePath,
			sizeof(telemetry_pack_data.FilePath)
		);

		memcpy(
			telemetry_pack_data.NewFilePath,
			data->NewFilePath,
			sizeof(telemetry_pack_data.NewFilePath)
		);

		FINAL_STRUCT_FILE_RENAME_EVENT final_data = { 0 };

		final_data.EventType = eventType;

		final_data.Size = sizeof(EVENT_FILE_RENAME_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_FILE_DELETE:
	{
		if (eventSize < sizeof(FILE_EVENT_SET_INFO_DATA))
		{
			return;
		}

		PFILE_EVENT_SET_INFO_DATA data =
			(PFILE_EVENT_SET_INFO_DATA)event;

		EVENT_FILE_DELETE_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.ThreadId =
			data->ThreadId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.Status =
			data->Status;

		telemetry_pack_data.InformationClass =
			data->InformationClass;

		telemetry_pack_data.RequestorMode =
			data->RequestorMode;

		telemetry_pack_data.DeleteFile =
			data->DeleteFile;

		memcpy(
			telemetry_pack_data.FilePath,
			data->FilePath,
			sizeof(telemetry_pack_data.FilePath)
		);

		FINAL_STRUCT_FILE_DELETE_EVENT final_data = { 0 };

		final_data.EventType = eventType;

		final_data.Size = sizeof(EVENT_FILE_DELETE_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_FILE_SET_INFORMATION:
	{
		if (eventSize < sizeof(FILE_EVENT_SET_INFO_DATA))
		{
			return;
		}

		PFILE_EVENT_SET_INFO_DATA data =
			(PFILE_EVENT_SET_INFO_DATA)event;

		EVENT_FILE_SET_INFORMATION_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.ThreadId =
			data->ThreadId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.Status =
			data->Status;

		telemetry_pack_data.InformationClass =
			data->InformationClass;

		telemetry_pack_data.RequestorMode =
			data->RequestorMode;

		memcpy(
			telemetry_pack_data.FilePath,
			data->FilePath,
			sizeof(telemetry_pack_data.FilePath)
		);

		FINAL_STRUCT_FILE_SET_INFORMATION_EVENT final_data = { 0 };

		final_data.EventType = eventType;

		final_data.Size = sizeof(EVENT_FILE_SET_INFORMATION_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	case EVENT_NETWORK_CONNECT:
	{
		if (eventSize < sizeof(NETWORK_EVENT_DATA))
		{
			return;
		}

		PNETWORK_EVENT_DATA data =
			(PNETWORK_EVENT_DATA)event;

		EVENT_NETWORK_CONNECT_PIPE telemetry_pack_data = { 0 };

		telemetry_pack_data.ProcessId =
			data->ProcessId;

		telemetry_pack_data.Timestamp =
			data->Timestamp;

		telemetry_pack_data.Protocol =
			data->Protocol;

		telemetry_pack_data.LocalAddress =
			data->LocalAddress;

		telemetry_pack_data.RemoteAddress =
			data->RemoteAddress;

		telemetry_pack_data.LocalPort =
			data->LocalPort;

		telemetry_pack_data.RemotePort =
			data->RemotePort;

		FINAL_STRUCT_NETWORK_CONNECT_EVENT final_data = { 0 };

		final_data.EventType = eventType;

		final_data.Size = sizeof(EVENT_NETWORK_CONNECT_PIPE);

		final_data.Data = telemetry_pack_data;

		SendEventToGui(
			&final_data,
			sizeof(final_data)
		);

		return;
	}

	default:
		return;
	}
}
