#include "Service.h"

HANDLE OpenMiniEDRDriver()
{
	HANDLE driver;

	driver = CreateFileW(
		DRIVER_NAME,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return driver;
}