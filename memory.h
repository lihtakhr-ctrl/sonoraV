#pragma once

namespace mem {
    uintptr_t get_module_base(
        const wchar_t* module_name);

    uintptr_t rel_to_abs(
        uintptr_t address,
        int offset,
        int instruction_size);

    uintptr_t pattern_scan(
        const char* pattern,
        uintptr_t module_base);

    template<typename t>
    t get_virtual_func(
        void* instance,
        size_t index)
    {
        if (!instance)
            return t{};

        auto vtable = *reinterpret_cast<uintptr_t**>(instance);
        if (!vtable)
            return t{};

        return reinterpret_cast<t>(vtable[index]);
    }

    template <typename t, typename ...Args>
    t call_virtual_func(
        void* instance,
        size_t index,
        Args... args)
    {
        using fn_t = t(__fastcall*)(Args...);
        auto fn = get_virtual_func<fn_t>(instance, index);
        if (!fn)
            return t{};

        return fn(args...);
    }
}