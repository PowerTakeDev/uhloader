#pragma once
#include <windows.h>
#include <ntdll.h>

#define LDR_IS_DATAFILE(handle)     (((ULONG_PTR)(handle)) & (ULONG_PTR)1)
#define MAXUSHORT                       0xffff

#undef SearchPath

#define ROUND_DOWN(n, align) \
    (((ULONG)n) & ~((align) - 1l))

#define ROUND_UP(n, align) \
    ROUND_DOWN(((ULONG)n) + (align) - 1, (align))

#define ROUND_SIZE(size) (max(16, ROUND_UP(size, 16)))

FORCEINLINE
PLARGE_INTEGER
WINAPI
BaseFormatTimeOut(
    PLARGE_INTEGER Timeout,
    DWORD dwMilliseconds
)
{
    /* Check if this is an infinite wait, which means no timeout argument */
    if (dwMilliseconds == 0xFFFFFFFF /* INFINITE */) return NULL;

    /* Otherwise, convert the time to NT Format */
    Timeout->QuadPart = dwMilliseconds * -10000LL;
    return Timeout;
}

FORCEINLINE
VOID
WINAPI
Sleep(
    DWORD dwMilliseconds
)
{
    PLARGE_INTEGER TimePtr;
    LARGE_INTEGER Time;

    TimePtr = BaseFormatTimeOut(&Time, dwMilliseconds);
    if (!TimePtr)
    {
        Time.LowPart = 0;
        Time.HighPart = 0x80000000;
        TimePtr = &Time;
    }

    NtDelayExecution(FALSE, TimePtr);
}

FORCEINLINE
_Ret_maybenull_
HANDLE
WINAPI
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
)
{
    return CreateRemoteThread(
        NtCurrentProcess,
        lpThreadAttributes,
        dwStackSize,
        lpStartAddress,
        lpParameter,
        dwCreationFlags,
        lpThreadId
    );
}

FORCEINLINE
int
WINAPI
MessageBoxExA(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType,
    WORD wLanguageId
)
{
    MSGBOXPARAMSA msgbox;

    msgbox.cbSize = sizeof(msgbox);
    msgbox.hwndOwner = hWnd;
    msgbox.hInstance = 0;
    msgbox.lpszText = lpText;
    msgbox.lpszCaption = lpCaption;
    msgbox.dwStyle = uType;
    msgbox.lpszIcon = NULL;
    msgbox.dwContextHelpId = 0;
    msgbox.lpfnMsgBoxCallback = NULL;
    msgbox.dwLanguageId = wLanguageId;

    return MessageBoxIndirectA(&msgbox);
}

FORCEINLINE
int
WINAPI
MessageBoxA(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
)
{
    return MessageBoxExA(hWnd, lpText, lpCaption, uType, LANG_NEUTRAL);
}

FORCEINLINE
LPVOID
NTAPI
VirtualAllocEx(IN HANDLE hProcess,
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect)
{
    NTSTATUS Status;

    /* Allocate the memory */
    Status = NtAllocateVirtualMemory(
        hProcess,
        &lpAddress,
        0,
        &dwSize,
        flAllocationType,
        flProtect
    );

    /* Check for status */
    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        return NULL;
    }

    /* Return the allocated address */
    return lpAddress;
}

FORCEINLINE
LPVOID
NTAPI
VirtualAlloc(IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect)
{
    /* Call the extended API */
    return VirtualAllocEx(GetCurrentProcess(),
        lpAddress,
        dwSize,
        flAllocationType,
        flProtect);
}

FORCEINLINE
BOOL
WINAPI
VirtualProtectEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
)
{
    NTSTATUS Status;

    /* Change the protection */
    Status = NtProtectVirtualMemory(
        hProcess,
        &lpAddress,
        &dwSize,
        flNewProtect,
        (PULONG)lpflOldProtect
    );

    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        return FALSE;
    }

    /* Return success */
    return TRUE;
}

FORCEINLINE
BOOL
WINAPI
VirtualProtect(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
)
{
    return VirtualProtectEx(GetCurrentProcess(),
        lpAddress,
        dwSize,
        flNewProtect,
        lpflOldProtect);
}

FORCEINLINE
BOOL
NTAPI
VirtualFreeEx(IN HANDLE hProcess,
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD dwFreeType)
{
    NTSTATUS Status;

    /* Validate size and flags */
    if (!(dwSize) || !(dwFreeType & MEM_RELEASE))
    {
        /* Free the memory */
        Status = NtFreeVirtualMemory(hProcess,
            &lpAddress,
            &dwSize,
            dwFreeType);
        if (!NT_SUCCESS(Status))
        {
            /* We failed */
            return FALSE;
        }

        /* Return success */
        return TRUE;
    }

    /* Invalid combo */
    return FALSE;
}

FORCEINLINE
BOOL
NTAPI
VirtualFree(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType
)
{
    /* Call the extended API */
    return VirtualFreeEx(GetCurrentProcess(),
        lpAddress,
        dwSize,
        dwFreeType);
}

FORCEINLINE
HANDLE
WINAPI
GetCurrentProcess(
    VOID
)
{
    return NtCurrentProcess;
}

FORCEINLINE
HANDLE
WINAPI
GetProcessHeap(
    VOID
)
{
    return NtCurrentPeb()->ProcessHeap;
}

FORCEINLINE
void*
malloc(
    size_t size
)
{
    size_t nSize = ROUND_SIZE(size);

    if (nSize < size)
        return NULL;

    return RtlAllocateHeap(GetProcessHeap(), 0, nSize);
}

FORCEINLINE
void*
calloc(
    size_t count,
    size_t size
)
{
    size_t nSize = count * size;
    size_t cSize = ROUND_SIZE(nSize);

    if ((count > ((size_t)-1 / size)) || (cSize < nSize))
        return NULL;

    return RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, cSize);
}

FORCEINLINE
void
free(void* ptr)
{
    RtlFreeHeap(GetProcessHeap(), 0, ptr);
}

FORCEINLINE
void*
memcpyA(
    void* dest,
    const void* src,
    size_t count
){
    char* char_dest = (char*)dest;
    char* char_src = (char*)src;

    if ((char_dest <= char_src) || (char_dest >= (char_src + count)))
    {
        while (count > 0)
        {
            *char_dest = *char_src;
            char_dest++;
            char_src++;
            count--;
        }
    }
    else
    {
        char_dest = (char*)dest + count - 1;
        char_src = (char*)src + count - 1;

        while (count > 0)
        {
            *char_dest = *char_src;
            char_dest--;
            char_src--;
            count--;
        }
    }

    return dest;
}

FORCEINLINE
PUNICODE_STRING
WINAPI
Basep8BitStringToStaticUnicodeString(IN LPCSTR String)
{
    PUNICODE_STRING StaticString = &(NtCurrentTeb()->StaticUnicodeString);
    ANSI_STRING AnsiString;

    /* Initialize an ANSI String */
    RtlInitAnsiString(&AnsiString, (PSTR)String);
    RtlAnsiStringToUnicodeString(StaticString, &AnsiString, FALSE);

    return StaticString;
}

FORCEINLINE
HINSTANCE
WINAPI
LoadLibraryExA(LPCSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags)
{
    PUNICODE_STRING FileNameW;

    /* Convert file name to unicode */
    if (!(FileNameW = Basep8BitStringToStaticUnicodeString(lpLibFileName)))
        return NULL;

    /* And call W version of the API */
    return LoadLibraryExW(FileNameW->Buffer, hFile, dwFlags);
}

FORCEINLINE
HINSTANCE
WINAPI
LoadLibraryA(LPCSTR lpLibFileName)
{
    /* Call the Ex version of the API */
    return LoadLibraryExA(lpLibFileName, 0, 0);
}

FORCEINLINE
void* __cdecl memsetA(void* src, int val, size_t count)
{
    char* char_src = (char*)src;

    while (count > 0) {
        *char_src = val;
        char_src++;
        count--;
    }
    return src;
}