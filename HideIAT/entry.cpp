#include <ntifs.h>
#include "HideIAT.hpp"

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	if (DriverObject) {
		DriverObject->DriverUnload = [](PDRIVER_OBJECT DriverObject)->VOID {
			UNREFERENCED_PARAMETER(DriverObject);
		};
	}
	for (ULONG i = 0; i < 5; i++)
	{
		Import(DbgPrint)("Hello! Called from hidden IAT! Time:%d\n", i);
	}
	return status;
}
