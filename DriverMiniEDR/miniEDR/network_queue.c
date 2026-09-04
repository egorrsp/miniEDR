#include "miniedr.h"
#include "network.h"

BOOLEAN PushNetworkConnectEvent(
	const FWPS_INCOMING_VALUES0* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* inMetaValues
)
{
	PNETWORK_EVENT event;

	event = ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(NETWORK_EVENT),
		'tENP'
	);

	if (event == NULL) {
		return FALSE;
	}

    RtlZeroMemory(
        event,
        sizeof(NETWORK_EVENT)
    );

    event->EventData.EventType =
        EVENT_NETWORK_CONNECT;

    KeQuerySystemTimePrecise(
        &event->EventData.Timestamp
    );

    if (FWPS_IS_METADATA_FIELD_PRESENT(
        inMetaValues,
        FWPS_METADATA_FIELD_PROCESS_ID))
    {
        event->EventData.ProcessId =
            (HANDLE)(ULONG_PTR)inMetaValues->processId;
    }

    event->EventData.Protocol =
        inFixedValues->incomingValue[
            FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL
        ].value.uint8;

    event->EventData.LocalAddress =
        inFixedValues->incomingValue[
            FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS
        ].value.uint32;

    event->EventData.RemoteAddress =
        inFixedValues->incomingValue[
            FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS
        ].value.uint32;

    event->EventData.LocalPort =
        inFixedValues->incomingValue[
            FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT
        ].value.uint16;

    event->EventData.RemotePort =
        inFixedValues->incomingValue[
            FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT
        ].value.uint16;

    return PushNetworkEventToQueue(event);
}

BOOLEAN PushNetworkEventToQueue(
    PNETWORK_EVENT event
)
{
	if (!PushEventToQueue(&event->ListEntry))
	{
		ExFreePool(event);
		return FALSE;
	}

	return TRUE;
}
