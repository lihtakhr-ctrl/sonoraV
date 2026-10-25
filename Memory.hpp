#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <string>
#include <vector>

/* ============================================================
 *  r5r はアンチチート無しなので RPM / WPM だけで十分
 * ============================================================ */
class Memory
{
public:
    ~Memory() { Close(); }

    bool Attach(const wchar_t* procName)
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);
        bool found = false;

        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, procName) == 0) {
                    pid_ = pe.th32ProcessID;
                    found = true;
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
        if (!found) return false;

        h_ = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE |
                         PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
                         FALSE, pid_);
        return h_ != nullptr;
    }

    void Close()
    {
        if (h_) { CloseHandle(h_); h_ = nullptr; }
    }

    bool ModuleBase(const wchar_t* name, uint64_t& base, uint64_t& size)
    {
        HANDLE snap = CreateToolhelp32Snapshot(
            TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid_);
        if (snap == INVALID_HANDLE_VALUE) return false;

        MODULEENTRY32W me{};
        me.dwSize = sizeof(me);
        bool found = false;

        if (Module32FirstW(snap, &me)) {
            do {
                if (_wcsicmp(me.szModule, name) == 0) {
                    base = (uint64_t)me.modBaseAddr;
                    size = me.modBaseSize;
                    found = true;
                    break;
                }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
        return found;
    }

    bool Read(uint64_t addr, void* out, size_t size)
    {
        if (!h_ || !addr || !size) return false;
        SIZE_T got = 0;
        return ReadProcessMemory(h_, (LPCVOID)addr, out, size, &got) != 0;
    }

    bool Write(uint64_t addr, const void* in, size_t size)
    {
        if (!h_ || !addr || !size) return false;
        SIZE_T put = 0;
        return WriteProcessMemory(h_, (LPVOID)addr, in, size, &put) != 0;
    }

    template <class T> T Read(uint64_t addr)
    {
        T v{};
        Read(addr, &v, sizeof(T));
        return v;
    }

    template <class T> void Write(uint64_t addr, const T& v)
    {
        Write(addr, &v, sizeof(T));
    }

    std::string ReadStr(uint64_t addr, size_t max = 32)
    {
        char b[64]{};
        if (max > sizeof(b)) max = sizeof(b);
        Read(addr, b, max);
        b[max - 1] = 0;
        return std::string(b);
    }

    DWORD  Pid() const { return pid_; }
    bool   Alive() const { return h_ != nullptr; }
    bool   IsOpen() const { return h_ != nullptr; }

private:
    HANDLE h_   = nullptr;
    DWORD  pid_ = 0;
};