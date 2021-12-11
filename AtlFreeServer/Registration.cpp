#include "pch.h"
#include <wrl/wrappers/corewrappers.h>
#include <cassert>
#include <ktmw32.h>
#pragma comment(lib, "ktmw32.lib")

struct RegistryKeyTraits
{
    using Type = HKEY;

    static bool Close(Type h) noexcept
    {
        return RegCloseKey(h) == ERROR_SUCCESS;
    }

    static Type GetInvalidValue() noexcept
    {
        return nullptr;
    }
};

using RegistryKey = Microsoft::WRL::Wrappers::HandleT<RegistryKeyTraits>;
using Transaction = Microsoft::WRL::Wrappers::HandleT<Microsoft::WRL::Wrappers::HandleTraits::HANDLENullTraits>;

Transaction CreateTransaction()
{
    return Transaction(CreateTransaction(nullptr, // default security descriptor
                                         nullptr, // reserved
                                         TRANSACTION_DO_NOT_PROMOTE,
                                         0, // reserved
                                         0, // reserved
                                         INFINITE,
                                         nullptr)); // description
}

RegistryKey CreateRegistryKey(HKEY key,
                              wchar_t const * path,
                              Transaction const & transaction,
                              REGSAM access)
{
    HKEY handle = nullptr;

    auto result = RegCreateKeyTransacted(key,
                                         path,
                                         0, // reserved
                                         nullptr, // class
                                         REG_OPTION_NON_VOLATILE,
                                         access,
                                         nullptr, // default security descriptor
                                         &handle,
                                         nullptr, // disposition
                                         transaction.Get(),
                                         nullptr); // reserved

    if (ERROR_SUCCESS != result)
    {
        SetLastError(result);
    }

    return RegistryKey(handle);
}

RegistryKey OpenRegistryKey(HKEY key,
                            wchar_t const * path,
                            Transaction const & transaction,
                            REGSAM access)
{
    HKEY handle = nullptr;

    auto result = RegOpenKeyTransacted(key,
                                       path,
                                       0, // reserved
                                       access,
                                       &handle,
                                       transaction.Get(),
                                       nullptr); // reserved

    if (ERROR_SUCCESS != result)
    {
        SetLastError(result);
    }

    return RegistryKey(handle);
}

enum class EntryOption
{
    None,     ///< Just add registry value. Typically sub-key that will be deleted by root key on uninstall
    Delete,   ///< Will be deleted during un-installation
    FileName, ///< Sets registry key default value to the path of this dll
};

struct Entry
{
    wchar_t const* Path; ///< Path to registry key
    EntryOption Option;

    wchar_t const* Name; ///< Name of value in registry
    wchar_t const* Value; ///< Value in registry
};

static Entry Table[] =
{
    // Register the dll 'AppID' allows the COM dll to run as an out of
    // process COM server under the dllhost.exe. This requires creating
    // a new guid for the AppID, and adding the DllSurrogate key. The
    // AppId is just another guid that registration of COM classes can
    // refer to as their AppID.
    {
        L"Software\\Classes\\AppID\\{2b083fea-3681-4c9b-9ed1-3e866124a58d}",
        EntryOption::Delete,
        nullptr,
        L"AtlFreeServer Object"
    },
    {
        L"Software\\Classes\\AppID\\{2b083fea-3681-4c9b-9ed1-3e866124a58d}",
        EntryOption::None,
        L"DllSurrogate",
        L""
    },

    // Registration of GuardDog COM class
    {
        // Adds the UID of the COM GuardDog class as key in the Registry
        L"Software\\Classes\\CLSID\\{d162d2f7-cdf4-44bc-8018-6058420bcfdc}",
        EntryOption::Delete,
        nullptr,
        L"GuardDog COM class"
    },
    {   // Associates the GuardDog with the AppID, and allows creating GuardDogs
        // in a separate process.
        L"Software\\Classes\\CLSID\\{d162d2f7-cdf4-44bc-8018-6058420bcfdc}",
        EntryOption::None,
        L"AppID",
        L"{2b083fea-3681-4c9b-9ed1-3e866124a58d}"
    },
    {   // Adds InprocServer32 subkey to GuardDog COM class with a default value containing the dll filename
        L"Software\\Classes\\CLSID\\{d162d2f7-cdf4-44bc-8018-6058420bcfdc}\\InprocServer32",
        EntryOption::FileName
    },
    {   // Adds another value to the InprocServer32 key
        L"Software\\Classes\\CLSID\\{d162d2f7-cdf4-44bc-8018-6058420bcfdc}\\InprocServer32",
        EntryOption::None,
        L"ThreadingModel",
        L"Free"
    },

    // Register the proxy dll CLSID. I think we can choose any guid, but the common way
    // is to use the first UID found in the IDog.idl file, namely the IDog uuid.
    // Interfaces will refer to this GUID to identify the dll containing the proxy/stub implementation.
    // Note that midl compiler can generate dedicated implementations of DllRegisterServer and
    // DllRegisterServer, but they are not suitable with merged proxy, and are not transactional
    {
        L"Software\\Classes\\CLSID\\{69fd604f-493c-4344-94b8-ea4179dd5113}",
        EntryOption::Delete,
        nullptr,
        L"GuardDog Proxy"
    },
    {
        L"Software\\Classes\\CLSID\\{69fd604f-493c-4344-94b8-ea4179dd5113}\\InprocServer32",
        EntryOption::FileName
    },
    {
        L"Software\\Classes\\CLSID\\{69fd604f-493c-4344-94b8-ea4179dd5113}\\InprocServer32",
        EntryOption::None,
        L"ThreadingModel",
        L"Both"
    },

    // Register the IDog interface. Interfaces go to 'Interface' key instead of 'CLSID'
    {
        L"Software\\Classes\\Interface\\{69fd604f-493c-4344-94b8-ea4179dd5113}",
        EntryOption::Delete,
        nullptr,
        L"IDog interface"
    },
    {
        L"Software\\Classes\\Interface\\{69fd604f-493c-4344-94b8-ea4179dd5113}\\ProxyStubClsid32",
        EntryOption::None,
        nullptr,
        L"{69fd604f-493c-4344-94b8-ea4179dd5113}" // Refer to the Proxy dll CLSID
    },

    // Register the IPostman interface since we will be using it via remoting
    {
        L"Software\\Classes\\Interface\\{d6ae480c-8b07-41f0-bea4-9eb3c7ed8d91}",
        EntryOption::Delete,
        nullptr,
        L"IPostman interface"
    },
    {
        L"Software\\Classes\\Interface\\{d6ae480c-8b07-41f0-bea4-9eb3c7ed8d91}\\ProxyStubClsid32",
        EntryOption::None,
        nullptr,
        L"{69fd604f-493c-4344-94b8-ea4179dd5113}" // Refer to the Proxy dll CLSID
    }
};

bool Unregister(Transaction const & transaction)
{
    for (auto const & entry : Table)
    {
        if (EntryOption::Delete != entry.Option)
        {
            continue;
        }

        auto key = OpenRegistryKey(HKEY_LOCAL_MACHINE,
                                   entry.Path,
                                   transaction,
                                   DELETE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE);

        if (!key.IsValid())
        {
            if (ERROR_FILE_NOT_FOUND == GetLastError())
            {
                continue;
            }

            return false;
        }

        auto result = RegDeleteTree(key.Get(),
                                    nullptr); // delete the key itself

        if (ERROR_SUCCESS != result)
        {
            SetLastError(result);
            return false;
        }
    }

    return true;
}

extern "C" IMAGE_DOS_HEADER __ImageBase; // Trickery to get path to current dll

bool Register(Transaction const & transaction)
{
    if (!Unregister(transaction))
    {
        return false;
    }

    wchar_t filename[MAX_PATH];

    auto const length = GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), //s_serverModule,
                                          filename,
                                          _countof(filename));

    if (0 == length || _countof(filename) == length)
    {
        return false;
    }

    for (auto const & entry : Table)
    {
        auto key = CreateRegistryKey(HKEY_LOCAL_MACHINE,
                                     entry.Path,
                                     transaction,
                                     KEY_WRITE);

        if (!key.IsValid())
        {
            return false;
        }

        if (EntryOption::FileName != entry.Option && !entry.Value)
        {
            continue;
        }

        auto value = entry.Value;

        if (!value)
        {
            assert(EntryOption::FileName == entry.Option);
            value = filename;
        }

        auto result = RegSetValueEx(key.Get(),
                                    entry.Name,
                                    0, // reserved
                                    REG_SZ,
                                    reinterpret_cast<BYTE const *>(value),
                                    static_cast<DWORD>(sizeof(wchar_t) * (wcslen(value) + 1)));

        if (ERROR_SUCCESS != result)
        {
            printf("RegSetValueEx failed %d\n", result);
            SetLastError(result);
            return false;
        }
    }

    return true;
}

HRESULT __stdcall DllRegisterServer()
{
    auto transaction = CreateTransaction();

    if (!transaction.IsValid())
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!Register(transaction))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CommitTransaction(transaction.Get()))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT __stdcall DllUnregisterServer()
{
    auto transaction = CreateTransaction();

    if (!transaction.IsValid())
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!Unregister(transaction))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CommitTransaction(transaction.Get()))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}
