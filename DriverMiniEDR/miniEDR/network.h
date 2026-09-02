#pragma once

#ifndef NDIS630
#define NDIS630 1
#endif

#include <ntddk.h>

#pragma warning(push)
#pragma warning(disable:4201)
#include <fwpsk.h>
#include <fwpmk.h>
#pragma warning(pop)

#include "network_types.h"

extern UINT32 g_NetworkCalloutId;

NTSTATUS InitNetworkFilter(
	PDEVICE_OBJECT DeviceObject
);

VOID CloseNetworkFilter(VOID);

NTSTATUS NTAPI NetworkNotify(
	FWPS_CALLOUT_NOTIFY_TYPE NotifyType,
	const GUID* FilterKey,
	FWPS_FILTER0* Filter
);

VOID NTAPI NetworkClassify(
	const FWPS_INCOMING_VALUES0* InFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* InMetaValues,
	VOID* LayerData,
	const FWPS_FILTER0* Filter,
	UINT64 FlowContext,
	FWPS_CLASSIFY_OUT0* ClassifyOut
);

BOOLEAN PushEventToQueue(
	PLIST_ENTRY Entry
);

BOOLEAN PushNetworkConnectEvent(
	const FWPS_INCOMING_VALUES0* InFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* InMetaValues
);

BOOLEAN PushNetworkEventToQueue(
	PNETWORK_EVENT Event
);
