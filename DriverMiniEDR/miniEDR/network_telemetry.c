#include "network.h"
#include <initguid.h>

UINT32 g_NetworkCalloutId = 0;

static HANDLE g_WfpEngineHandle = NULL;
static UINT32 g_ManagementCalloutId = 0;
static UINT64 g_NetworkFilterId = 0;

// {E613A731-8712-4C46-9B6C-FB1D7C914738}
DEFINE_GUID(
    MINI_EDR_NETWORK_CALLOUT_GUID,
    0xe613a731,
    0x8712,
    0x4c46,
    0x9b, 0x6c,
    0xfb, 0x1d, 0x7c, 0x91, 0x47, 0x38
);

// {61B43AF0-2E9B-4A57-AF2F-12B6FD780A61}
DEFINE_GUID(
    MINI_EDR_NETWORK_SUBLAYER_GUID,
    0x61b43af0,
    0x2e9b,
    0x4a57,
    0xaf, 0x2f,
    0x12, 0xb6, 0xfd, 0x78, 0x0a, 0x61
);

static NTSTATUS RegisterRuntimeCallout(
    PDEVICE_OBJECT DeviceObject
)
{
    FWPS_CALLOUT0 callout = { 0 };

    callout.calloutKey = MINI_EDR_NETWORK_CALLOUT_GUID;
    callout.classifyFn = NetworkClassify;
    callout.notifyFn = NetworkNotify;

    return FwpsCalloutRegister0(
        DeviceObject,
        &callout,
        &g_NetworkCalloutId
    );
}

static NTSTATUS RegisterBfeObjects(VOID)
{
    NTSTATUS status;
    FWPM_SESSION0 session = { 0 };
    FWPM_SUBLAYER0 subLayer = { 0 };
    FWPM_CALLOUT0 callout = { 0 };
    FWPM_FILTER0 filter = { 0 };

    session.displayData.name = L"miniEDR dynamic WFP session";
    session.flags = FWPM_SESSION_FLAG_DYNAMIC;

    status = FwpmEngineOpen0(
        NULL,
        RPC_C_AUTHN_WINNT,
        NULL,
        &session,
        &g_WfpEngineHandle
    );
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = FwpmTransactionBegin0(g_WfpEngineHandle, 0);
    if (!NT_SUCCESS(status))
    {
        FwpmEngineClose0(g_WfpEngineHandle);
        g_WfpEngineHandle = NULL;
        return status;
    }

    subLayer.subLayerKey = MINI_EDR_NETWORK_SUBLAYER_GUID;
    subLayer.displayData.name = L"miniEDR network telemetry sublayer";
    subLayer.weight = 0x100;

    status = FwpmSubLayerAdd0(
        g_WfpEngineHandle,
        &subLayer,
        NULL
    );
    if (!NT_SUCCESS(status))
    {
        goto AbortTransaction;
    }

    callout.calloutKey = MINI_EDR_NETWORK_CALLOUT_GUID;
    callout.displayData.name = L"miniEDR outbound connect callout";
    callout.applicableLayer = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

    status = FwpmCalloutAdd0(
        g_WfpEngineHandle,
        &callout,
        NULL,
        &g_ManagementCalloutId
    );
    if (!NT_SUCCESS(status))
    {
        goto AbortTransaction;
    }

    filter.displayData.name = L"miniEDR outbound IPv4 telemetry";
    filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    filter.subLayerKey = MINI_EDR_NETWORK_SUBLAYER_GUID;
    filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
    filter.action.calloutKey = MINI_EDR_NETWORK_CALLOUT_GUID;
    filter.weight.type = FWP_EMPTY;

    status = FwpmFilterAdd0(
        g_WfpEngineHandle,
        &filter,
        NULL,
        &g_NetworkFilterId
    );
    if (!NT_SUCCESS(status))
    {
        goto AbortTransaction;
    }

    status = FwpmTransactionCommit0(g_WfpEngineHandle);
    if (!NT_SUCCESS(status))
    {
        FwpmTransactionAbort0(g_WfpEngineHandle);
        FwpmEngineClose0(g_WfpEngineHandle);
        g_WfpEngineHandle = NULL;
        g_ManagementCalloutId = 0;
        g_NetworkFilterId = 0;
    }

    return status;

AbortTransaction:
    FwpmTransactionAbort0(g_WfpEngineHandle);
    FwpmEngineClose0(g_WfpEngineHandle);
    g_WfpEngineHandle = NULL;
    g_ManagementCalloutId = 0;
    g_NetworkFilterId = 0;
    return status;
}

VOID NTAPI NetworkClassify(
    const FWPS_INCOMING_VALUES0* inFixedValues,
    const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    VOID* layerData,
    const FWPS_FILTER0* filter,
    UINT64 flowContext,
    FWPS_CLASSIFY_OUT0* classifyOut
)
{
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);

    if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) != 0)
    {
        classifyOut->actionType = FWP_ACTION_CONTINUE;
    }

    if (inFixedValues == NULL || inMetaValues == NULL ||
        inFixedValues->layerId != FWPS_LAYER_ALE_AUTH_CONNECT_V4)
    {
        return;
    }

    if (!PushNetworkConnectEvent(inFixedValues, inMetaValues))
    {
        DbgPrintEx(
            DPFLTR_IHVDRIVER_ID,
            DPFLTR_WARNING_LEVEL,
            "miniEDR: network event was dropped\n"
        );
    }
}

NTSTATUS NTAPI NetworkNotify(
    FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    const GUID* filterKey,
    FWPS_FILTER0* filter
)
{
    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);

    return STATUS_SUCCESS;
}

NTSTATUS InitNetworkFilter(
    PDEVICE_OBJECT DeviceObject
)
{
    NTSTATUS status;

    if (DeviceObject == NULL)
    {
        return STATUS_INVALID_DEVICE_STATE;
    }

    status = RegisterRuntimeCallout(DeviceObject);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = RegisterBfeObjects();
    if (!NT_SUCCESS(status))
    {
        FwpsCalloutUnregisterById0(g_NetworkCalloutId);
        g_NetworkCalloutId = 0;
    }

    return status;
}

VOID CloseNetworkFilter(VOID)
{
    NTSTATUS status;

    if (g_WfpEngineHandle != NULL)
    {
        FwpmEngineClose0(g_WfpEngineHandle);
        g_WfpEngineHandle = NULL;
        g_ManagementCalloutId = 0;
        g_NetworkFilterId = 0;
    }

    if (g_NetworkCalloutId != 0)
    {
        status = FwpsCalloutUnregisterById0(g_NetworkCalloutId);
        if (!NT_SUCCESS(status))
        {
            DbgPrintEx(
                DPFLTR_IHVDRIVER_ID,
                DPFLTR_ERROR_LEVEL,
                "miniEDR: FwpsCalloutUnregisterById0 failed: 0x%08X\n",
                status
            );
            return;
        }

        g_NetworkCalloutId = 0;
    }
}
