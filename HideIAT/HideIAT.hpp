#include <ntifs.h>
#include <ntimage.h>

namespace HideIAT
{
    PVOID GetKernelBase()
    {
        auto IdtBase = *(ULONG64*)(__readgsqword(0x18) + 0x38);     // x64 kernel mode only
        auto Start = *(ULONG64*)(IdtBase + 4) & 0xFFFFFFFFFFFF0000;
        for (auto Page = (PUCHAR)Start; Page > (PUCHAR)Start - 0xB00000; Page -= 0x1000) {
            for (int i = 0; i < 0xFF9; ++i) {
                if (*(USHORT*)&Page[i] == 0x8D48 && Page[i + 2] == 0x1D && Page[i + 6] == 0xFF) {
                    auto KernelBase = &Page[i] + 7 + *(int*)&Page[i + 3];
                    if (((ULONG64)KernelBase & 0xFFF) == 0)
                        return KernelBase;
                }
            }
        }
        return NULL;
    }

    PVOID GetModuleExport(PVOID ImageBase, PCCHAR Name)
    {
        ULONG_PTR FunctionAddress = 0;
        PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ImageBase;
        PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)ImageBase + DosHeader->e_lfanew);
        PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)ImageBase);
        PUSHORT AddressOfNameOrdinals = (PUSHORT)(ExportDirectory->AddressOfNameOrdinals + (ULONG_PTR)ImageBase);
        PULONG AddressOfNames = (PULONG)(ExportDirectory->AddressOfNames + (ULONG_PTR)ImageBase);
        PULONG AddressOfFunctions = (PULONG)(ExportDirectory->AddressOfFunctions + (ULONG_PTR)ImageBase);
        for (ULONG i = 0; i < ExportDirectory->NumberOfFunctions; ++i) {
            PCHAR  pName = (PCHAR)(AddressOfNames[i] + (ULONG_PTR)ImageBase);
            if (!strcmp(pName, Name)) {
                FunctionAddress = AddressOfFunctions[AddressOfNameOrdinals[i]] + (ULONG_PTR)ImageBase;
            }
        }
        return (PVOID)FunctionAddress;
    }

    PVOID KernelBase = 0;
    PVOID GetKernelExport(PCCHAR NameOrd)
    {
        if (!KernelBase) {
            KernelBase = GetKernelBase();
        }
        return GetModuleExport(KernelBase, NameOrd);
    }
}

#define Import(name)	((decltype(&##name))(HideIAT::GetKernelExport(#name)))
