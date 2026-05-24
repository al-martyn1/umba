/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

#include "umba/umba.h"
#include "umba/filename.h"
#include "umba/filesys.h"
//
#include <vector>
#include <string>
//

#if !defined(WIN32) && !defined(_WIN32)
    #error "This file is only for Win32/Win64"
#endif



//----------------------------------------------------------------------------
// umba::win32_utils::
// umba::win32::
namespace umba {
namespace win32 {

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
bool isWindows32OnWindows64()
{

//#if defined(WIN64) || defined(_WIN64)

//    return false; // 64хбитное приложение, нет проблем

//#else

    // https://stackoverflow.com/questions/14184137/how-can-i-determine-whether-a-process-is-32-or-64-bit

    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    BOOL bIsWow64 = FALSE;

    auto hKernel = GetModuleHandle(TEXT("kernel32"));
    if (hKernel)
    {
        // disable_C4191.h
        // disable_unsafe_conversion.h
        #include "umba/warnings/push_disable_unsafe_conversion.h"
        LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(hKernel,"IsWow64Process");
        #include "umba/warnings/pop.h"
        if (fnIsWow64Process)
        {
            if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
            {
                //handle error
            }
        }
    }

    return bIsWow64 ? true : false;

//#endif
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template<typename StringType>
std::vector<StringType> makeUninstallRegPathsVector()
{
    std::vector<StringType> v;
    v.emplace_back(umba::string_plus::make_string<StringType>("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"));  // SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall
    v.emplace_back(umba::string_plus::make_string<StringType>("SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall")); // SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall
    return v;
}

//----------------------------------------------------------------------------
template<typename StringType>
const std::vector<StringType>& getUninstallRegPathsVector()
{
    static auto v = makeUninstallRegPathsVector<StringType>();
    return v;
}

//----------------------------------------------------------------------------
inline
HKEY regCreateKey(HKEY hKeyRoot, const std::wstring &path, REGSAM samDesired)
{
    // https://learn.microsoft.com/ru-ru/windows/win32/winprog64/accessing-an-alternate-registry-view
    // Не надо тут делать, пусть юзер сам думает
    // if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
    // {
    //     samDesired |= KEY_WOW64_64KEY;
    // }

    HKEY hKeyRes = 0;
    DWORD dwDisposition = 0;

    LSTATUS status = RegCreateKeyExW( hKeyRoot
                                    , path.c_str()
                                    , 0 // reserved
                                    , 0 // lpClass - The user-defined class type of this key. This parameter may be ignored. This parameter can be NULL.
                                    , REG_OPTION_NON_VOLATILE // default, 0
                                    , samDesired
                                    , 0 // lpSecurityAttributes
                                    , &hKeyRes
                                    , &dwDisposition
                                    );
    if (status!=ERROR_SUCCESS)
    {
        return 0;
    }

    return hKeyRes;

}

//----------------------------------------------------------------------------
inline
HKEY regCreateKey(HKEY hKeyRoot, const std::string &path, REGSAM samDesired)
{
    // Не надо тут делать, пусть юзер сам думает
    // if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
    // {
    //     samDesired |= KEY_WOW64_64KEY;
    // }

    HKEY hKeyRes = 0;
    DWORD dwDisposition = 0;

    LSTATUS status = RegCreateKeyExA( hKeyRoot
                                    , path.c_str()
                                    , 0 // reserved
                                    , 0 // lpClass - The user-defined class type of this key. This parameter may be ignored. This parameter can be NULL.
                                    , REG_OPTION_NON_VOLATILE // default, 0
                                    , samDesired
                                    , 0 // lpSecurityAttributes
                                    , &hKeyRes
                                    , &dwDisposition
                                    );
    if (status!=ERROR_SUCCESS)
    {
        return 0;
    }

    return hKeyRes;

}

//----------------------------------------------------------------------------
inline
LSTATUS regOpenKeyEx( HKEY              hKey
                    , const std::string &subKeyName
                    , DWORD             ulOptions
                    , REGSAM            samDesired
                    , PHKEY             phkResult
                    )
{
    // https://learn.microsoft.com/ru-ru/windows/win32/winprog64/accessing-an-alternate-registry-view
    // Не надо тут делать, пусть юзер сам думает
    // if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
    // {
    //     samDesired |= KEY_WOW64_64KEY;
    // }

    return RegOpenKeyExA(hKey, subKeyName.c_str(), ulOptions, samDesired, phkResult);
}

//----------------------------------------------------------------------------
inline
LSTATUS regOpenKeyEx( HKEY               hKey
                    , const std::wstring &subKeyName
                    , DWORD              ulOptions
                    , REGSAM             samDesired
                    , PHKEY              phkResult
                    )
{
    // https://learn.microsoft.com/ru-ru/windows/win32/winprog64/accessing-an-alternate-registry-view
    // Не надо тут делать, пусть юзер сам думает
    // if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
    // {
    //     samDesired |= KEY_WOW64_64KEY;
    // }

    return RegOpenKeyExW(hKey, subKeyName.c_str(), ulOptions, samDesired, phkResult);
}

//----------------------------------------------------------------------------
inline
bool regQueryValueType(HKEY hKey, const std::string &valueName, DWORD &type, LSTATUS *pStatus=0)
{
    LSTATUS st = RegQueryValueExA(hKey, valueName.c_str(), 0, &type, 0, 0);
    if (pStatus)
       *pStatus = st;
    return st==ERROR_SUCCESS;
}

//----------------------------------------------------------------------------
inline
bool regQueryValueType(HKEY hKey, const std::wstring &valueName, DWORD &type, LSTATUS *pStatus=0)
{
    LSTATUS st = RegQueryValueExW(hKey, valueName.c_str(), 0, &type, 0, 0);
    if (pStatus)
       *pStatus = st;
    return st==ERROR_SUCCESS;
}

//----------------------------------------------------------------------------
inline
bool regQueryValueEx( HKEY                        hKey
                    , const std::wstring          &valueName
                    , std::vector<std::uint8_t>   &data
                    , DWORD                       *pType=0
                    , LSTATUS                     *pStatus=0
                    )
{
    DWORD type   = 0;
    DWORD cbData = 0;

    data.clear();

    LSTATUS 
    status = RegQueryValueExW( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)0
                             , &cbData
                             );
    if (pStatus)
       *pStatus = status;

    if (status!=ERROR_SUCCESS && status!=ERROR_MORE_DATA)
        return false;

    if (pType)
        *pType = type;

    if (cbData==0)
        return true;
    
    data.resize(std::size_t(cbData), 0);

    status = RegQueryValueExW( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)&data[0]
                             , &cbData
                             );
    if (pStatus)
       *pStatus = status;

    if (pType)
        *pType = type;

    if (status!=ERROR_SUCCESS && status!=ERROR_MORE_DATA)
        return false;

    return true;
}

//----------------------------------------------------------------------------
inline
bool regQueryValueEx( HKEY                        hKey
                    , const std::string           &valueName
                    , std::vector<std::uint8_t>   &data
                    , DWORD                       *pType=0
                    , LSTATUS                     *pStatus=0
                    )
{
    DWORD type   = 0;
    DWORD cbData = 0;

    data.clear();

    LSTATUS 
    status = RegQueryValueExA( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)0
                             , &cbData
                             );
    if (pStatus)
       *pStatus = status;

    if (pType)
        *pType = type;

    if (status!=ERROR_SUCCESS && status!=ERROR_MORE_DATA)
        return false;

    if (cbData==0)
        return true;

    data.resize(std::size_t(cbData), 0);

    status = RegQueryValueExA( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)&data[0]
                             , &cbData
                             );
    if (pStatus)
       *pStatus = status;

    if (pType)
        *pType = type;

    if (status!=ERROR_SUCCESS && status!=ERROR_MORE_DATA)
        return false;

    return true;
}

//----------------------------------------------------------------------------
inline
bool regQueryValueEx( HKEY               hKey
                    , const std::wstring &valueName
                    , std::wstring       &value
                    , DWORD              *pType=0
                    , LSTATUS            *pStatus=0
                    )
{
    wchar_t buf[1024];
    DWORD type;
    DWORD cbData = sizeof(buf); // in bytes

    wchar_t *pBuf = &buf[0];

    LSTATUS 
    status = RegQueryValueExW( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)pBuf
                             , &cbData
                             );

    std::vector<wchar_t> dynBuf;
    if (status==ERROR_MORE_DATA)
    {
        std::size_t needChars = cbData/sizeof(buf[0]);
        if (cbData%sizeof(buf[0]))
            ++needChars;
        dynBuf.resize(needChars, 0);
        pBuf = dynBuf.data();
    }
    else if (status!=ERROR_SUCCESS)
    {
        if (pStatus)
           *pStatus = status;
        return false;
    }

    status = RegQueryValueExW( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)pBuf
                             , &cbData
                             );

    if (status!=ERROR_SUCCESS)
    {
        if (pStatus)
           *pStatus = status;
        return false;
    }

    if (type!=REG_SZ && type!=REG_EXPAND_SZ)
    {
        if (pStatus)
           *pStatus = ERROR_DATATYPE_MISMATCH;
        return false;
    }
    
    if (pType)
        *pType = type;

    std::size_t charsCopied = cbData/sizeof(buf[0]);
    if (cbData%sizeof(buf[0]))
        ++charsCopied;

    if (charsCopied>0 && pBuf[charsCopied-1]==0)
        --charsCopied; // strip terminating zero

    value.assign(pBuf, charsCopied);

    return true;
}

//----------------------------------------------------------------------------
inline
bool regQueryValueEx( HKEY               hKey
                    , const std::string  &valueName
                    , std::string        &value
                    , DWORD              *pType=0
                    , LSTATUS            *pStatus=0
                    )
{
    char  buf[1024];
    DWORD type;
    DWORD cbData = sizeof(buf); // in bytes

    char *pBuf = &buf[0];

    LSTATUS 
    status = RegQueryValueExA( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)pBuf
                             , &cbData
                             );

    std::vector<char> dynBuf;
    if (status==ERROR_MORE_DATA)
    {
        std::size_t needChars = cbData/sizeof(buf[0]);
        // if (cbData%sizeof(buf[0]))
        //     ++needChars;
        dynBuf.resize(needChars, 0);
        pBuf = dynBuf.data();
    }
    else if (status!=ERROR_SUCCESS)
    {
        if (pStatus)
           *pStatus = status;
        return false;
    }

    status = RegQueryValueExA( hKey
                             , valueName.c_str()
                             , 0 // reserved
                             , &type
                             , (LPBYTE)pBuf
                             , &cbData
                             );

    if (status!=ERROR_SUCCESS)
    {
        if (pStatus)
           *pStatus = status;
        return false;
    }

    if (type!=REG_SZ && type!=REG_EXPAND_SZ)
    {
        if (pStatus)
           *pStatus = ERROR_DATATYPE_MISMATCH;
        return false;
    }
    
    if (pType)
        *pType = type;

    std::size_t charsCopied = cbData/sizeof(buf[0]);
    if (cbData%sizeof(buf[0]))
        ++charsCopied;

    if (charsCopied>0 && pBuf[charsCopied-1]==0)
        --charsCopied; // strip terminating zero

    value.assign(pBuf, charsCopied);

    return true;
}

//----------------------------------------------------------------------------
template<typename StringType> inline
bool regGetValue( HKEY                        hKey
                , const StringType            &subKeyName
                , const StringType            &valueName
                , std::vector<std::uint8_t>   &data
                , REGSAM                      samDesired = 0 // extra flags
                , DWORD                       *pType=0
                , LSTATUS                     *pStatus=0
                )
{
    HKEY subKey;

    samDesired |= KEY_QUERY_VALUE;
    
    LSTATUS
    status = regOpenKeyEx( hKey, subKeyName
                         , 0 // ulOptions
                         , samDesired
                         , &subKey
                         );

    if (pStatus)
       *pStatus = status;

    if (status!=ERROR_SUCCESS)
        return false;

    try
    {
        bool res = regQueryValueEx(subKey, valueName, data, pType, pStatus);
        RegCloseKey(subKey);
        return res;
    }
    catch(...)
    {
        RegCloseKey(subKey);
        throw;
    }
}

//----------------------------------------------------------------------------
template<typename StringType> inline
bool regGetValue( HKEY                        hKey
                , const StringType            &subKeyName
                , const StringType            &valueName
                , StringType                  &data
                , REGSAM                      samDesired = 0 // extra flags
                , DWORD                       *pType=0
                , LSTATUS                     *pStatus=0
                )
{
    HKEY subKey;

    samDesired |= KEY_QUERY_VALUE;
    
    LSTATUS
    status = regOpenKeyEx( hKey, subKeyName
                         , 0 // ulOptions
                         , samDesired
                         , &subKey
                         );

    if (pStatus)
       *pStatus = status;

    if (status!=ERROR_SUCCESS)
        return false;

    try
    {
        bool res = regQueryValueEx(subKey, valueName, data, pType, pStatus);
        RegCloseKey(subKey);
        return res;
    }
    catch(...)
    {
        RegCloseKey(subKey);
        throw;
    }
}

//----------------------------------------------------------------------------
inline
bool regEnumValue( HKEY         hKey
                 , DWORD        dwIndex
                 , std::string  &name
                 , DWORD        &dwType
                 , LSTATUS      *pStatus=0
                 )
{
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
    char buf[260+12];
    DWORD cchValueName = sizeof(buf)/sizeof(buf[0]);

    LSTATUS st = RegEnumValueA( hKey
                              , dwIndex
                              , &buf[0]
                              , &cchValueName
                              , 0 // lpReserved
                              , &dwType
                              , 0 // lpData
                              , 0 // lpcbData
                              );
    if (st!=ERROR_SUCCESS)
    {
        if (pStatus)
           *pStatus = st;
        return false;
    }

    name.assign(&buf[0], cchValueName);

    if (pStatus)
       *pStatus = ERROR_SUCCESS;

    return true;
}

//----------------------------------------------------------------------------
inline
bool regEnumValue( HKEY         hKey
                 , DWORD        dwIndex
                 , std::wstring &name
                 , DWORD        &dwType
                 , LSTATUS      *pStatus=0
                 )
{
    // https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
    wchar_t buf[16384];
    DWORD cchValueName = sizeof(buf)/sizeof(buf[0]);

    LSTATUS st = RegEnumValueW( hKey
                              , dwIndex
                              , &buf[0]
                              , &cchValueName
                              , 0 // lpReserved
                              , &dwType
                              , 0 // lpData
                              , 0 // lpcbData
                              );
    if (st!=ERROR_SUCCESS)
    {
        if (pStatus)
           *pStatus = st;
        return false;
    }

    name.assign(&buf[0], cchValueName);

    if (pStatus)
       *pStatus = ERROR_SUCCESS;

    return true;
}

//----------------------------------------------------------------------------
template<typename StringType>
struct RegValueInfo
{
    StringType     name;
    DWORD          type;

}; // struct RegValueInfo

//----------------------------------------------------------------------------
template<typename StringType>
inline
void regEnumValues(HKEY hKey, std::vector< RegValueInfo<StringType> > &valInfos, LSTATUS *pStatus=0)
{
    // if (pStatus)
    //    *pStatus = ERROR_SUCCESS;

    DWORD dwIndex = 0;
    for(;;++dwIndex)
    {
        RegValueInfo<StringType> valInfo;
        if (!regEnumValue(hKey, dwIndex, valInfo.name, valInfo.type, pStatus))
            break;

        valInfos.emplace_back(valInfo);
    }
}

//----------------------------------------------------------------------------
inline
bool regSetValue(HKEY hKey, const std::string &name, const std::string &value, LSTATUS *pStatus=0, bool expandSz=false)
{
    LSTATUS st = RegSetValueExA( hKey, name.c_str()
                               , 0 // Reserved
                               , expandSz ? REG_EXPAND_SZ : REG_SZ
                               , (const BYTE*)value.c_str()
                               , (DWORD)(value.size()+1)
                               );
    if (pStatus)
       *pStatus = st;

    return st==ERROR_SUCCESS;
}

//----------------------------------------------------------------------------
inline
bool regSetValue(HKEY hKey, const std::wstring &name, const std::wstring &value, LSTATUS *pStatus=0, bool expandSz=false)
{
    LSTATUS st = RegSetValueExW( hKey, name.c_str()
                               , 0 // Reserved
                               , expandSz ? REG_EXPAND_SZ : REG_SZ
                               , (const BYTE*)value.c_str()
                               , (DWORD)((value.size()+1)*sizeof(wchar_t))
                               );
    if (pStatus)
       *pStatus = st;

    return st==ERROR_SUCCESS;
}

//----------------------------------------------------------------------------
template<typename StringType>
bool regQueryAppInstallLocation(const StringType &appUninstallSectionName, StringType &foundPath)
{
    REGSAM samDesired = KEY_READ;
    if (isWindows32OnWindows64()) // 32х-битные системы сейчас конечно уже экзотика, но на всякий случай - я же и на XP могу работать
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    const auto& regPaths = getUninstallRegPathsVector<StringType>();

    for(const auto &rp: regPaths)
    {
        HKEY hk = 0;
        LSTATUS
        status = regOpenKeyEx( HKEY_LOCAL_MACHINE
                             , rp + umba::string_plus::make_string<StringType>("\\") + appUninstallSectionName
                             , 0 // ulOptions
                             , samDesired
                             , &hk
                             );
        if (status==ERROR_SUCCESS)
        {
            if (regQueryValueEx(hk, umba::string_plus::make_string<StringType>("InstallLocation"), foundPath))
            {
                RegCloseKey(hk);
                umba::string_plus::unquote( foundPath, '\"', '\"');
                return true;
            }

            if (regQueryValueEx(hk, umba::string_plus::make_string<StringType>("UninstallString"), foundPath))
            {
                RegCloseKey(hk);

                umba::string_plus::unquote( foundPath, '\"', '\"');
                foundPath = umba::filename::getPath(foundPath);
                return true;
            }

            RegCloseKey(hk);
        }
    }

    return false;
}

//----------------------------------------------------------------------------
template<typename StringType>
bool regQueryAppInstallLocationBin(const StringType &appUninstallSectionName, StringType &foundPath)
{
    if (regQueryAppInstallLocation(appUninstallSectionName, foundPath))
    {
        if (umba::filesys::isPathDirectory(umba::filename::appendPath(foundPath, string_plus::make_string<StringType>("bin"))))
        {
            foundPath = umba::filename::appendPath(foundPath, string_plus::make_string<StringType>("bin"));
        }

        return true;
    }

    return false;
}


} // namespace win32

namespace win32_utils = win32;

} // namespace umba



