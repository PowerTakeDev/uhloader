#include <windows.h>
#include <dbg.h>
#include <KeyValues.h>
#include <utlvector.h>
#include "winapi.h"

#include "subhook/subhook.h"
#include "GameEventManager.h"
#include "netmessages.h"

void __fastcall Hooked_WriteListenEventList(CGameEventManager* pThis, void* EDX, CLC_ListenEvents* msg)
{
    msg->m_EventArray.ClearAll();

    int nCount = *((DWORD*)pThis + 4);

    CGameEventDescriptor* pGameEvents = (CGameEventDescriptor*)*((DWORD*)pThis + 1);

    for (int i = 0; i < nCount; i++)
    {
        CGameEventDescriptor& descriptor = pGameEvents[i];

        bool bHasClientListener = false;

        for (int j = 0; j < descriptor.listeners.Count(); j++)
        {
            CGameEventCallback* pListener = descriptor.listeners[j];

            if (pListener->m_nListenerType == CGameEventManager::CLIENTSIDE ||
                pListener->m_nListenerType == CGameEventManager::CLIENTSIDE_OLD)
            {
                bHasClientListener = true;
                break;
            }
        }

        if (!bHasClientListener)
            continue;

        if (descriptor.eventid == -1)
        {
            continue;
        }
        
        if (!strcmp(descriptor.name, "player_say") || !strcmp(descriptor.name, "player_hurt"))
        {
            continue;
        }

        msg->m_EventArray.Set(descriptor.eventid);
    }
}

typedef BOOL(WINAPI* FreeLibraryFn)(
    HMODULE hLibModule
);
FreeLibraryFn FreeLibrary_;

HMODULE g_hModule = NULL;
HMODULE g_hPayload = NULL;
BOOL g_bWork = TRUE;

BOOL CALLBACK Hooked_FreeLibrary(HMODULE hModule)
{
    if (hModule == g_hPayload)
    {
        g_bWork = FALSE;
    }

    return FreeLibrary_(hModule);
}

subhook_t g_pWriteListenEventListHook;
subhook_t g_pFreeLibraryHook;

FORCEINLINE
BOOL
InstallHooks()
{
    DWORD dwEngine = (DWORD)GetModuleHandleA("engine.dll");
    if (dwEngine == NULL)
    {
        return FALSE;
    }

    void* pWriteListenEventList = reinterpret_cast<void*>(dwEngine + 0xADA80);
    if (pWriteListenEventList == NULL)
    {
        return FALSE;
    }

    g_pWriteListenEventListHook = subhook_new(pWriteListenEventList, (void*)Hooked_WriteListenEventList, (subhook_flags_t)0);
    if (subhook_install(g_pWriteListenEventListHook) != 0)
    {
        return FALSE;
    }
    
    g_pFreeLibraryHook = subhook_new(FreeLibrary, (void*)Hooked_FreeLibrary, (subhook_flags_t)0);
    if (subhook_install(g_pFreeLibraryHook) != 0)
    {
        return FALSE;
    }

    FreeLibrary_ = reinterpret_cast<FreeLibraryFn>( subhook_get_trampoline(g_pFreeLibraryHook) );
    if (FreeLibrary_ == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

FORCEINLINE
VOID
UninstallHooks()
{
    subhook_remove(g_pWriteListenEventListHook);
    subhook_remove(g_pFreeLibraryHook);
}

DWORD CALLBACK DllThread(LPVOID lpArgs)
{
    WCHAR wszFileName[512];
    GetModuleFileNameW(g_hModule, wszFileName, 500);

    for (size_t i = 500; i > 0; i--)
    {
        if (wszFileName[i] == '\\')
        {
            wszFileName[i + 1] = 'u';
            wszFileName[i + 2] = 'h';
            wszFileName[i + 3] = '.';
            wszFileName[i + 4] = 'd';
            wszFileName[i + 5] = 'l';
            wszFileName[i + 6] = 'l';
            wszFileName[i + 7] = '\0';
            break;
        }
    }

    if (InstallHooks() == FALSE)
    {
        FreeLibraryAndExitThread(g_hModule, EXIT_FAILURE);
    }

    g_hPayload = LoadLibraryExW(wszFileName, NULL, 0);
    if (g_hPayload == NULL)
    {
        WCHAR wszBuffer[1024];
        memcpyA(wszBuffer, L"Failed to load library \"", 47);
        size_t i = 24;
        for (; wszFileName[i - 24] != '\0'; i++)
        {
            wszBuffer[i] = wszFileName[i - 24];
        }
        wszBuffer[i] = '\"';
        wszBuffer[i + 1] = '\0';
        MessageBoxW(NULL, wszBuffer, L"Loader - Error", MB_OK | MB_ICONERROR);
        FreeLibraryAndExitThread(g_hModule, EXIT_FAILURE);
    }

    while (g_bWork)
    {
        Sleep(1);
    }

	FreeLibraryAndExitThread(g_hModule, EXIT_SUCCESS);
}

BOOL WINAPI EntryPoint(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
        g_hModule = hModule;
        LdrDisableThreadCalloutsForDll(hModule);
		CreateThread(NULL, NULL, DllThread, hModule, 0, NULL);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        UninstallHooks();
    }

	return TRUE;
}