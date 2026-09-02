#pragma once

#include <fltKernel.h>
#include "network_types.h"

#define MINI_EDR_PATH_CAPACITY 256

NTSTATUS InitTelemetry(VOID);

NTSTATUS ProcessWatcher(VOID);
NTSTATUS FileMappingWatcher(VOID);

// Event encoding
#define EVENT_PROCESS_CLOSE ((ULONG)0)
#define EVENT_PROCESS_CREATE ((ULONG)1)

#define EVENT_FILE_MAPPED ((ULONG)3)

#define EVENT_FILE_CREATE ((ULONG)4)
#define EVENT_FILE_WRITE ((ULONG)5)
#define EVENT_FILE_RENAME ((ULONG)6)
#define EVENT_FILE_DELETE ((ULONG)7)
#define EVENT_FILE_SET_INFORMATION ((ULONG)8)

/// Queue structures of telemetry events

// Process create/close
typedef struct _PROCESS_EVENT_DATA
{
	ULONG EventType;

	HANDLE ProcessId;
	HANDLE ProcessParentId;

	LARGE_INTEGER Timestamp;

	WCHAR ImagePath[MINI_EDR_PATH_CAPACITY];

} PROCESS_EVENT_DATA, * PPROCESS_EVENT_DATA;

typedef struct _PROCESS_EVENT
{
	LIST_ENTRY ListEntry;

	PROCESS_EVENT_DATA EventData;

} PROCESS_EVENT, * PPROCESS_EVENT;


// File mapping / unmapping in memory
typedef struct _FILE_EVENT_DATA
{
	ULONG EventType;
	HANDLE ProcessId;

	LARGE_INTEGER Timestamp;

	PVOID ImageBase;
	SIZE_T ImageSize;

	ULONG SystemModeImage;
	ULONG ImageSignatureLevel;
	ULONG ImageSignatureType;

	WCHAR ImagePath[MINI_EDR_PATH_CAPACITY];

} FILE_EVENT_DATA, * PFILE_EVENT_DATA;

typedef struct _FILE_EVENT
{
	LIST_ENTRY ListEntry;

	FILE_EVENT_DATA EventData;

} FILE_EVENT, *PFILE_EVENT;


// File changing - CREATE
typedef struct _FILE_CREATE_EVENT_DATA {

	ULONG EventType;

	HANDLE ProcessId;
	HANDLE ThreadId;

	LARGE_INTEGER Timestamp;

	NTSTATUS Status;

	ACCESS_MASK DesiredAccess;
	ULONG CreateDisposition;
	ULONG CreateOptions;
	ULONG FileAttributes;
	ULONG ShareAccess;

	ULONG RequestorMode;

	WCHAR FilePath[MINI_EDR_PATH_CAPACITY];

} FILE_CREATE_EVENT_DATA, *PFILE_CREATE_EVENT_DATA;

typedef struct _FILE_CREATE_EVENT {

	LIST_ENTRY ListEntry;

	FILE_CREATE_EVENT_DATA EventData;

} FILE_CREATE_EVENT, *PFILE_CREATE_EVENT;

// File changing - WRITE
typedef struct _FILE_WRITE_EVENT_DATA
{
	ULONG EventType;

	HANDLE ProcessId;
	HANDLE ThreadId;

	LARGE_INTEGER Timestamp;

	NTSTATUS Status;

	ULONG RequestedLength;
	ULONG_PTR BytesWritten;

	LARGE_INTEGER ByteOffset;

	ULONG RequestorMode;

	WCHAR FilePath[MINI_EDR_PATH_CAPACITY];

} FILE_WRITE_EVENT_DATA, *PFILE_WRITE_EVENT_DATA;

typedef struct _FILE_WRITE_EVENT {

	LIST_ENTRY ListEntry;

	FILE_WRITE_EVENT_DATA EventData;

} FILE_WRITE_EVENT, *PFILE_WRITE_EVENT;


//File changing - Set Information
typedef struct _FILE_EVENT_SET_INFO_DATA {
	ULONG EventType;

	HANDLE ProcessId;
	HANDLE ThreadId;

	LARGE_INTEGER Timestamp;

	NTSTATUS Status;

	FILE_INFORMATION_CLASS InformationClass;
	ULONG RequestorMode;

	BOOLEAN DeleteFile;
	BOOLEAN ReplaceIfExists;

	WCHAR FilePath[MINI_EDR_PATH_CAPACITY];
	WCHAR NewFilePath[MINI_EDR_PATH_CAPACITY];

} FILE_EVENT_SET_INFO_DATA, *PFILE_EVENT_SET_INFO_DATA;

typedef struct _FILE_EVENT_SET_INFO {

	LIST_ENTRY ListEntry;

	FILE_EVENT_SET_INFO_DATA EventData;

} FILE_EVENT_SET_INFO, *PFILE_EVENT_SET_INFO;

// Filter for I/O in files
NTSTATUS InitFileFilter(PDRIVER_OBJECT);
VOID CloseFileFilter(VOID);

// Filter for network activity
NTSTATUS InitNetworkFilter(PDEVICE_OBJECT);
VOID CloseNetworkFilter(VOID);

extern PFLT_FILTER g_Filter;

VOID ProcessNotifyCallback(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

VOID FileNotifyCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
);

VOID CloseTelemetry(VOID);
