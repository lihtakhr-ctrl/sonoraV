#include "core.h"

uintptr_t mem::get_module_base(const wchar_t* module_name) {
    PEB* peb = nullptr;
#ifdef _WIN64
    peb = reinterpret_cast<PEB*>(__readgsqword(0x60));
#else
    peb = reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif

    if (!peb || !peb->Ldr || !peb->Ldr->InMemoryOrderModuleList.Flink) {
        return 0;
    }

    auto head = &peb->Ldr->InMemoryOrderModuleList;
    auto current = head->Flink;

    struct MY_LDR_DATA_TABLE_ENTRY {
        LIST_ENTRY InLoadOrderLinks;
        LIST_ENTRY InMemoryOrderLinks;
        LIST_ENTRY InInitializationOrderLinks;
        PVOID DllBase;
        PVOID EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING FullDllName;
        UNICODE_STRING BaseDllName;
    };

    while (current != head) {
        auto entry = CONTAINING_RECORD(current, MY_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (module_name == nullptr)
            return reinterpret_cast<uintptr_t>(entry->DllBase);

        if (entry->BaseDllName.Buffer) {
            std::wstring base_name(
                entry->BaseDllName.Buffer,
                entry->BaseDllName.Length / sizeof(wchar_t)
            );

            if (_wcsicmp(module_name, base_name.c_str()) == 0) {
                return reinterpret_cast<uintptr_t>(entry->DllBase);
            }
        }

        current = current->Flink;
    }

    return 0;
}
uintptr_t mem::rel_to_abs(
    uintptr_t address,
    int offset,
    int instruction_size)
{
    const auto instruction_address = address + offset;
    const int relative_offset = *reinterpret_cast<int*>(instruction_address);
    return address + instruction_size + relative_offset;
}

uintptr_t mem::pattern_scan(
    const char* pattern,
    uintptr_t module_base)
{
    static auto pattern_to_bytes = [](const char* pattern) {
        auto       bytes = std::vector<int>{};
        const auto start = const_cast<char*>(pattern);
        const auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
        };

    if (!module_base)
        return 0;

    const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<uint8_t*>(module_base) + dos_header->e_lfanew);

    if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    const auto image_size = nt_headers->OptionalHeader.SizeOfImage;
    const auto pattern_bytes = pattern_to_bytes(pattern);
    const auto scan_base = reinterpret_cast<uint8_t*>(module_base);
    const auto pattern_size = pattern_bytes.size();
    const auto pattern_data = pattern_bytes.data();

    for (size_t i = 0; i < image_size - pattern_size; ++i) {
        bool found = true;

        for (size_t j = 0; j < pattern_size; ++j) {
            if (scan_base[i + j] != pattern_data[j] && pattern_data[j] != -1) {
                found = false;
                break;
            }
        }

        if (found)
            return reinterpret_cast<uintptr_t>(&scan_base[i]);
    }

    return 0;
}
