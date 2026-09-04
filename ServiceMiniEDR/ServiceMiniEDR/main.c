#include "Service.h"

int wmain(void)
{
    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        { L"miniEDRService", ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcherW(serviceTable);

    return 0;
}
