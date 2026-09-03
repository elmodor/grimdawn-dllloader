#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cstdint>

constexpr size_t PROC_TABLE_SIZE = 256;
extern "C" {
    void* proc_table[PROC_TABLE_SIZE];
}

static void Log(const char* fmt, ...)
{
#ifndef NOLOG
    FILE* f = fopen("GDDllLoader.log", "a");
    if(!f)
        return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02u:%02u:%02u.%03u] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    fprintf(f, "\n");
    va_end(args);
    fclose(f);
#endif
}

void ResolveExports()
{
    Log("Resolving exports");

    HMODULE realWinmm = LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
    if (realWinmm)
        Log("Real winmm loaded");
    else
    {
        Log("Real winmm failed to load");
        return;
    }

    const char* names[] =
    {
#include "export_names.inc"
    };
    constexpr size_t names_count = sizeof(names) / sizeof(names[0]);
    static_assert(names_count <= PROC_TABLE_SIZE);
    for (int i = 0; i < names_count; i++)
    {
        if(names[i])
        {
            proc_table[i] = (void*)GetProcAddress(realWinmm, names[i]);
            if (!proc_table[i])
                Log("Failed: %s", names[i]);
            else
                Log("%s = %p", names[i], proc_table[i]);
        }
    }
}

struct LoadDllParams
{
    const char* name;
    bool reqGameDlls;
    uint32_t delay;
    uint32_t tries;
};

void LoadDll(const char* name, bool reqGameDlls, uint32_t delay = 0, uint32_t tries = 1)
{
    auto* params = static_cast<LoadDllParams*>(VirtualAlloc(nullptr, sizeof(LoadDllParams), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    if (!params)
    {
        Log("Virtual alloc failed: %lu", GetLastError());
        return;
    }

    params->name = name;
    params->reqGameDlls = reqGameDlls;
    params->delay = delay;
    params->tries = tries;

    HANDLE thread = CreateThread(nullptr, 0,
            [](LPVOID arg) -> DWORD
            {
                auto* params = static_cast<LoadDllParams*>(arg);
                if (params->reqGameDlls)
                {
                    while (!GetModuleHandleA("d3d11.dll"))
                        Sleep(10);
                    while (!GetModuleHandleA("Game.dll"))
                        Sleep(10);
                    while (!GetModuleHandleA("Engine.dll"))
                        Sleep(10);
                }
                Sleep(params->delay);
                Log("Loading %s", params->name);
                HMODULE mod = nullptr;
                for (uint32_t i = 0; i < params->tries; ++i)
                {
                    mod = LoadLibraryA(params->name);
                    if (!mod)
                        Log("Loading %s failed %lu", params->name, GetLastError());
                    Sleep(500);
                    if (mod && GetModuleHandleA(params->name))
                        break;
                    if (mod)
                        FreeLibrary(mod);
                    mod = nullptr;
                    if (i + 1 < params->tries)
                        Log("Retrying to load %s", params->name);
                }
                if (mod)
                    Log("%s loaded OK", params->name);
                else
                    Log("%s failed", params->name);
                VirtualFree(params, 0, MEM_RELEASE);
                return 0;
            }, params, 0, nullptr);

    if (!thread)
    {
        Log("Thread creation failed: %lu", GetLastError());
        VirtualFree(params, 0, MEM_RELEASE);
        return;
    }
    CloseHandle(thread);
}


BOOL WINAPI DllMain(HINSTANCE hinstance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstance);

#ifndef NOLOG
        remove("GDDllLoader.log");
#endif

        LoadDll("dpyes.dll", false, 0, 4);
        LoadDll("RiftgateCompanion.dll", false, 100, 1);
        LoadDll("ItemAssistantHook_x64.dll", true, 500, 1);

        ResolveExports();
    }

    return TRUE;
}
