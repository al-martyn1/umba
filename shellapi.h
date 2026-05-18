/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

#include "umba.h"
#include "filesys.h"
#include "utf8.h"
#include "string_plus.h"
#include "filename.h"
#include "filesys.h"
#include "enum_helpers.h"

//
#include <cstdint>
#include <set>
#include <unordered_set>

#if defined(WIN32) || defined(_WIN32)

    #include <shellapi.h>
    #include <processthreadsapi.h>

    #if defined(_MSC_VER)
        #pragma comment( lib, "Shell32" )
        // https://ru.stackoverflow.com/questions/15077/%D0%98%D1%81%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-pragma-comment
    #endif

    #include "win32_utils.h"
    #include "winconhelpers.h"
    //
    #include "internal/winstrerrorname.h"
    #include "internal/filesys.h"
    #include "filename.h"

    //
    #include <process.h>
    #include <stdio.h>
    #include <wchar.h>

#else

    #include <errno.h>
    #include <string.h>
    #include <unistd.h>

#endif

#include <cerrno>


//----------------------------------------------------------------------------
// umba::shellapi::
namespace umba {
namespace shellapi {

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
int getLastError()
{
#if defined(WIN32) || defined(_WIN32)
    return (int)GetLastError();
#else
    return errno;
#endif
}

//------------------------------
//! Возвращает строку с сообщением об ошибке, если задано addCodeValue, то также добавляется числовое значение ошибки
inline
std::string getErrorMessage(int errCode = getLastError(), bool addCodeValue=true)
{
#if defined(WIN32) || defined(_WIN32)

    // https://learn.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages

    WCHAR wszMsgBuff[256];
    DWORD dwChars = ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
                                    , NULL, (DWORD)errCode, 0
                                    , wszMsgBuff, 256, NULL
                                    );
    if (dwChars==0)
    {
        HINSTANCE hInst = ::LoadLibraryW(L"Ntdsbmsg.dll");
        if (hInst!=0)
        {
            dwChars = ::FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS
                                      , hInst, (DWORD)errCode, 0
                                      , wszMsgBuff, 256,NULL
                                      );
            ::FreeLibrary( hInst );
        }
    }

    std::size_t msgLen = std::min((std::size_t)255, (std::size_t)dwChars);
    wszMsgBuff[msgLen] = 0;

    std::string msgOnlyStr = toUtf8(wszMsgBuff);

    const char *pCodeStr = 0;
    {
        //const std::unordered_map<unsigned, const char*>& 
        const auto &m = win32::getStrErrorNameMap();
        auto it = m.find((unsigned)errCode);
        if (it!=m.end())
        {
            pCodeStr = it->second;
        }
    }

#else
    
    // https://man7.org/linux/man-pages/man3/strerror.3.html
    // https://man7.org/linux/man-pages/man7/feature_test_macros.7.html

    const char *pCodeStr = 0;
    #if defined(_GNU_SOURCE)
    {
        pCodeStr = strerrorname_np(errCode);
    }
    #endif

    std::string msgOnlyStr;

    #if defined(_REENTRANT) || defined(_THREAD_SAFE)
        char buf[256] = {0};
        strerror_r(errCode, &buf[0], 256);
        buf[255] = 0;
        msgOnlyStr = buf;
    #else
        auto pStr = strerror(errCode);
        if (pStr)
            msgOnlyStr = pStr;
    #endif

#endif

    // if (pCodeStr)
    //     codeStr = pCodeStr;


    std::string msgStr;
    if (pCodeStr && !msgOnlyStr.empty())
    {
        msgStr = std::string(pCodeStr) + " - " + msgOnlyStr;
    }
    else if (pCodeStr)
    {
        msgStr = pCodeStr;
    }
    else
    {
        msgStr = msgOnlyStr;
    }

    for(auto &ch : msgStr)
    {
        //ch = (ch<' ') ? ' ' : ch;
        if (ch>0 && ch<' ')
            ch = ' ';
    }

    msgStr = string_plus::rtrim_copy(msgStr);


    if (addCodeValue)
    {
        #if defined(WIN32) || defined(_WIN32)
            std::string errCodeValueStr = std::to_string((unsigned)errCode);
        #else
            std::string errCodeValueStr = std::to_string(errCode);
        #endif

         if (msgStr.empty())
             msgStr = errCodeValueStr;
         else
             msgStr += " (" + errCodeValueStr + ")";
    }

    return msgStr;
}

//----------------------------------------------------------------------------





 // WCHAR   wszMsgBuff[512];  // Buffer for text.
 //  
 //    DWORD   dwChars;  // Number of chars returned.
 //  
 //    // Try to get the message from the system errors.
 //    dwChars = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
 //                             FORMAT_MESSAGE_IGNORE_INSERTS,
 //                             NULL,
 //                             dwErr,
 //                             0,
 //                             wszMsgBuff,
 //                             512,
 //                             NULL );



//----------------------------------------------------------------------------
//! Удаляет каталог со всем содержимым. path - в UTF-8
inline
bool deleteDirectory(const std::string &path)
{
#if defined(WIN32) || defined(_WIN32)

    std::wstring wp = filename::makeCanonical(filesys::encodeToNative(path));

    SHFILEOPSTRUCTW  shFileOpStruct = {};
    wp.append(1, (wchar_t)0); // Надо два нуля в конце, потому что функция принимает разделяемый нулём список строк, и двойной ноль - окончание списка
    
    shFileOpStruct.hwnd   = 0;
    shFileOpStruct.wFunc  = FO_DELETE;
    shFileOpStruct.pFrom  = (PCZZWSTR)wp.c_str();
    shFileOpStruct.pTo    = 0;
    shFileOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;  // FOF_NOERRORUI - FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR
    shFileOpStruct.fAnyOperationsAborted = 0;
    shFileOpStruct.hNameMappings         = 0;
    shFileOpStruct.lpszProgressTitle     = 0;
    
    return SHFileOperationW(&shFileOpStruct)==0 ? true : false;

#else

    // !!!
    return false;

#endif
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)
inline
bool shellExecuteImpl(const std::wstring &verb, const std::string &documentFileName, bool showMaximized=false)
{

    // https://learn.microsoft.com/ru-ru/windows/win32/shell/launch
    // https://learn.microsoft.com/en-us/windows/win32/shell/launch
    // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea
    auto res = (INT_PTR)ShellExecuteW( 0, verb.c_str(), fromUtf8(documentFileName).c_str(), 0, 0, showMaximized ? SW_SHOWMAXIMIZED : SW_SHOW );
    return res>32; // If the function succeeds, it returns a value greater than 32
}
#endif

//----------------------------------------------------------------------------
inline
bool executeOpen(const std::string &documentFileName, bool showMaximized=false)
{
#if defined(WIN32) || defined(_WIN32)
    return shellExecuteImpl(L"open", documentFileName, showMaximized);
#else
    return false;
#endif
}

//----------------------------------------------------------------------------
//! Открывает заданный URL в браузере по умолчанию
inline
bool openUrl(const std::string &url, bool showMaximized=false)
{
#if defined(WIN32) || defined(_WIN32)
    return shellExecuteImpl(L"open", url, showMaximized);
#else
    return false;
#endif
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
enum class MoveFileFlags
{
    copyAllowed     = 1, //!< If the file is to be moved to a different volume, the function simulates the move by using the CopyFile and DeleteFile functions.
    replaceExisting = 2, //!< If a file named lpNewFileName exists, the function replaces its contents with the contents of the lpExistingFileName file
    overwrite       = 2, //!< Same as replaceExisting
    writeThrough    = 4  //!< The function does not return until the file is actually moved on the disk. Setting this value guarantees that a move performed as a copy and delete operation is flushed to disk before the function returns. The flush occurs at the end of the copy operation.
};

UMBA_ENUM_CLASS_IMPLEMENT_BIT_OPERATORS(MoveFileFlags)
UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_BIT_OPERATORS(MoveFileFlags)
UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_EQUAL_OPERATORS(MoveFileFlags)

//------------------------------
inline bool moveFile(const std::string &existingFileName, const std::string &newFileName, MoveFileFlags flags)
{
#if defined(WIN32) || defined(_WIN32)

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa
    std::wstring existingFileNameW = filename::makeCanonical(filesys::encodeToNative(existingFileName));
    std::wstring newFileNameW      = filename::makeCanonical(filesys::encodeToNative(newFileName));

    DWORD dwFlags = 0;

    if ((flags&MoveFileFlags::copyAllowed    )!=0)
        dwFlags |= MOVEFILE_COPY_ALLOWED;
    if ((flags&MoveFileFlags::replaceExisting)!=0)
        dwFlags |= MOVEFILE_REPLACE_EXISTING;
    if ((flags&MoveFileFlags::writeThrough   )!=0)
        dwFlags |= MOVEFILE_WRITE_THROUGH;

    return MoveFileExW(existingFileNameW.c_str(), newFileNameW.c_str(), dwFlags)!=0;

#else

    return false;

#endif
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Возвращает имя текущего исполняемого файла с полным путём
inline
std::string getCurrentProcessExecutableFileName()
{
#if defined(WIN32) || defined(_WIN32)

    wchar_t nameBuf[4096]; // в неупоротой системе не может быть таких путей
    DWORD dwRes = GetModuleFileNameW( 0 // for preocess itself
                                    , &nameBuf[0]
                                    , 4095
                                    );
    UMBA_USED(dwRes);
    nameBuf[4095] = 0; // кладём ноль в последний элемент массива, чтобы не париться

    return toUtf8(nameBuf);

#else

    return "qqq"; // !!! тут нужен замут через dl*, но пока лень и не особо нужно

#endif
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::string getUmbaTempLogRoot()
{
    std::string tmpRoot        = filesys::getTempFolderPath();
    std::string tmpRootUmba    = filename::appendPath(tmpRoot, std::string(".umba"));
    std::string tmpRootUmbaLog = filename::appendPath(tmpRootUmba, std::string("log"));
    return tmpRootUmbaLog;
}

//------------------------------
inline
std::string getUmbaTempLogAppPath()
{
    auto exeName = getCurrentProcessExecutableFileName();
    auto appName = filename::getName(exeName);
    return filename::appendPath(getUmbaTempLogRoot(), appName);
}

//------------------------------
//! Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
inline
std::string getUmbaTempLogNowFileName( const std::string &suffix, std::string fileNameFormat=std::string())
{
    if (fileNameFormat.empty())
        fileNameFormat = "%Y-%m-%d_%H-%M-%S"; // safe name for file system
//        fileNameFormat = "%Y %m %d %H:%M:%S";

    static std::string ftLastStr;
    static std::size_t eventCounter = 0;

    auto logPath  = getUmbaTempLogAppPath();
    auto ftNow    = filesys::getFileTimeNow();
    auto ftNowStr = filesys::formatFiletime(ftNow, fileNameFormat);
    if (ftLastStr!=ftNowStr)
    {
        ftLastStr = ftNowStr;
        eventCounter = 0;
    }
    std::string eventCounterStr = std::to_string(eventCounter+1); ++eventCounter;
    if (eventCounterStr.size()<4)
    {
        eventCounterStr = std::string(4-eventCounterStr.size(), '0') + eventCounterStr;
    }

    auto name     = suffix.empty() ? ftNowStr : ftNowStr + "_" + eventCounterStr + "_" + suffix;
    name          = filename::appendExt(name, std::string("log"));
    auto res      = filename::appendPath(logPath, name);
    return res;
}

//------------------------------
//! Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
inline
void writeUmbaEventLogNow( const std::string &eventName, std::string eventMsg, std::string fileNameFormat=std::string())
{
    UMBA_USED(fileNameFormat);
    if (eventMsg.empty())
        return;

    // !!!

    auto logName = getUmbaTempLogNowFileName(eventName);
    auto logPath = filename::getPath(logName);
    filesys::createDirectoryEx(logPath, true /* forceCreatePath */);
    auto wrRes = filesys::writeFile(logName, eventMsg, true /* bOverwrite */ );
    UMBA_USED(wrRes);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Производит экранирование одиночного аргумента для вызова в командной строке.
inline
std::string escapeCommandLineArgument(const std::string &str)
{

#if defined(WIN32) || defined(_WIN32)

    // http://learn.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments?view=msvc-170

    bool needEscape = false;
    if (str.find_first_of(" \"")!=str.npos)
       needEscape = true;

    if (!needEscape)
        return str;

    //using CharType = typename StringType::value_type;

    std::string res; res.reserve(str.size());
    res.append(1, '\"');
    for(auto ch : str)
    {
        if (ch=='\"')
            res.append(2, '\"');
        else
            res.append(1, ch);
    }
    res.append(1, '\"');

    return res;

#else

    return str; // Для *nix - пока ничего не делаем, надо разбираться

#endif

}

//----------------------------------------------------------------------------
//! Делает единственную строку из имени команды и аргументов, экранирует, и прочее
inline
std::string makeSystemFunctionCommandString(const std::string &cmd, std::vector<std::string> cmdArgs)
{
    if (!cmd.empty())
        cmdArgs.insert(cmdArgs.begin(), cmd);

    for(auto &cmdArg : cmdArgs)
    {
        cmdArg = escapeCommandLineArgument(cmdArg);
    }

    //using CharType = typename StringType::value_type;

    return umba::string_plus::merge<std::string, typename std::vector<std::string>::const_iterator>( cmdArgs.begin(), cmdArgs.end(), ' '/*, [](auto s) { return s; }*/ );
}

//----------------------------------------------------------------------------
//! Делает единственную строку без имени команды, из аргументов, экранирует, и прочее
inline
std::string makeSystemFunctionCommandString(std::vector<std::string> cmdArgs)
{
    for(auto &cmdArg : cmdArgs)
    {
        cmdArg = escapeCommandLineArgument(cmdArg);
    }

    return umba::string_plus::merge<std::string, typename std::vector<std::string>::const_iterator>( cmdArgs.begin(), cmdArgs.end(), ' '/*, [](auto s) { return s; }*/ );
}

//----------------------------------------------------------------------------
//! Делает единственную строку из имени команды, без аргументов, экранирует, и прочее
inline
std::string makeSystemFunctionCommandString(const std::string &cmd)
{
    return escapeCommandLineArgument(cmd);
}

//----------------------------------------------------------------------------
//! Производит вызов функции "system". Командная строка должна быть соответственно подготовлена (экранирование и тп)
inline
int callSystem(const std::string &cmd, std::string *pErrMsg=0, bool allocateConsole=true)
{
    // system returns the value that is returned by the command interpreter.
    // It returns the value 0 only if the command interpreter returns the value 0.
    // A return value of -1 indicates an error, and errno is set

    // https://lastpixel.tv/win32-application-as-both-gui-and-console/
    // https://stackoverflow.com/questions/48444176/win32-gui-application-compiled-as-gui-need-to-use-a-console-in-c
    // https://learn.microsoft.com/ru-ru/windows/console/attachconsole
    // BOOL WINAPI AttachConsole( _In_ DWORD dwProcessId);

    // Нет разницы, использовать ли AttachConsole, и при ошибке - AllocConsole, или сразу использовать AllocConsole.
    // Если используем AllocConsole, то она не откроет новое консольное окно, если мы уже прикреплены к консоли.
    // AllocConsole нужно использовать, потому, что если мы используем функцию system без этого, то она на каждый запуск
    // создаёт консоль, и потом закрывает. Это плохо, и антивири могут ругаться, когда из одного приложения много таких
    // окон выскакивает.
    // Это годно для GUI программ, которые не долго работают, но часто вызывают 'system'.
    // Для GUI программ, которые делают это редко, а работают долго - ситуация наоборот - не надо постоянно держать открытую консоль
    // Так что, пожалуй, сделаем это опцией

#if defined(WIN32) && defined(_WIN32)

    if (allocateConsole)
        AllocConsole();

    int resVal = _wsystem(fromUtf8(cmd).c_str());

#else

    int resVal = system(cmd.c_str());

#endif

    if (resVal!=0)
    {
        if (pErrMsg)
        {
            if (resVal==-1)
            {
                #include "umba/warnings/push_disable_fn_or_var_unsafe.h"
                *pErrMsg = std::string("Launch command failed: ") + std::strerror(errno);
                #include "umba/warnings/pop.h"
            }
            else
            {
                *pErrMsg = "Command result code: " + std::to_string(resVal);
            }
        }
    }

    return resVal;
}

//----------------------------------------------------------------------------
inline
int callSystem(const std::string &cmd, const std::vector<std::string> &cmdArgs, std::string *pErrMsg=0, bool allocateConsole=true)
{
    return callSystem(makeSystemFunctionCommandString(cmd, cmdArgs), pErrMsg, allocateConsole);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
/*

    Поиск исполняемых файлов.

    where /R C:\ chrome.exe
    dir /s C:\chrome.exe

    https://chat.deepseek.com/share/bmqxjikfbbr1il3ejh

    HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
    HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths


*/


//----------------------------------------------------------------------------
enum class SpawnProcessFlags
{
    default_               = 0x00,

    noUseExactExeName      = 0x00, // do lookup in PATH env var
    useExactExeName        = 0x01, // no lookup in PATH env var

    noArgsExeAlready       = 0x00,
    argsExeAlready         = 0x02, // executable file name already added to cmdArgs

};

UMBA_ENUM_CLASS_IMPLEMENT_BIT_OPERATORS(SpawnProcessFlags)
UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_BIT_OPERATORS(SpawnProcessFlags)
UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_EQUAL_OPERATORS(SpawnProcessFlags)

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Запускает процесс без ожидания, в отличие от callSystem. Также не используется никакой shell
// Поиск производится в путях, переменные среды наследуются от текущего процесса
// Возвращает PID процесса
inline
int spawnProcess(const std::string &cmd, const std::vector<std::string> &cmdArgs, SpawnProcessFlags spawnProcessFlags=SpawnProcessFlags::default_, std::string *pErrMsg=0)
{
    // UMBA_ARG_USED(allocateConsole); // , bool allocateConsole=true

    #if defined(WIN32) && defined(_WIN32)

        // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/spawnvp-wspawnvp?view=msvc-170
        // https://learn.microsoft.com/en-us/cpp/c-runtime-library/spawn-wspawn-functions?view=msvc-170
        // _P_NOWAIT
        std::vector<std::wstring> argsWide; argsWide.reserve(cmdArgs.size()+1);

        if ((spawnProcessFlags&SpawnProcessFlags::argsExeAlready)==0)
        {
            argsWide.push_back(fromUtf8(cmd));
        }

        for(auto &&a : cmdArgs)
        {
            argsWide.push_back(fromUtf8(a));
        }

        std::vector<const wchar_t*> argv; argv.reserve(argsWide.size()+1);
        for(const auto& wa : argsWide)
        {
            argv.push_back(wa.c_str());
        }

        argv.push_back(nullptr);

        auto wCmd = fromUtf8(cmd);

        const bool usePath = (spawnProcessFlags&SpawnProcessFlags::useExactExeName)==0;
        auto pSpawnFn = usePath ? &_wspawnvp : &_wspawnv;

        // Если системная пременная PATH содержит много путей, то данная функция обламывается, и возвращает EINVAL,
        // что выглядит весьма непрозрачно, об этом надо просто знать

        std::intptr_t processHandle = pSpawnFn( _P_NOWAIT // |_P_DETACH
                                              , wCmd.c_str()
                                              , &argv[0]
                                              );
        if (processHandle==-1)
        {
            if (pErrMsg)
            {
                #include "umba/warnings/push_disable_fn_or_var_unsafe.h"
                *pErrMsg =  /* std::string("Launch command failed: ") +  */ std::strerror(errno);
                #include "umba/warnings/pop.h"
            }

            return -1;
        }

        return 0;


//         DWORD GetProcessId(
//   [in] HANDLE Process
// );

    #else

        // posix_spawn
        // Standard C library (libc, -lc)
        // #include <spawn.h> ?
        // https://man7.org/linux/man-pages/man3/posix_spawn.3.html
        // https://pubs.opengroup.org/onlinepubs/9799919799/functions/posix_spawn.html
        // https://www.opennet.ru/man.shtml?topic=posix_spawn&category=3&russian=5
        //
        // !!! open O_CLOEXEC

        // https://chat.deepseek.com/share/vdbbm4xb6b8u4v6j9q

    #endif

}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
enum class MessageBoxKind
{
     mbDefault        = 0,
     iconNone         = 0,

     iconExclamation  = 1,  // !
     iconWarning      = 1,  // !

     iconAsterisk     = 2,  // i
     iconInformation  = 2,  // i

     iconQuestion     = 3,  // ?

     iconStop         = 4,  // [X]
     iconError        = 4   // [X]

};

//------------------------------
//! Отображает блокирующее выполнение GUI сообщение (Message Box), параметры - в UTF-8
inline
void showMessageBox(const std::string &msg, std::string title=std::string(), MessageBoxKind mbKind=MessageBoxKind::mbDefault)
{

    #if defined(WIN32) || defined(_WIN32)

        UINT uType = MB_OK|MB_SYSTEMMODAL; // MB_APPLMODAL;

        switch(mbKind)
        {
            case MessageBoxKind::mbDefault:
            {
                if (title.empty())
                    title = "Message";
                break;
            }

            case MessageBoxKind::iconExclamation: // aka iconWarning
            {
                if (title.empty())
                    title = "Warning";
                uType |= MB_ICONWARNING;
                break;
            }
            case MessageBoxKind::iconAsterisk: // aka iconInformation
            {
                if (title.empty())
                    title = "Information";
                uType |= MB_ICONASTERISK;
                break;
            }
            case MessageBoxKind::iconQuestion:
            {
                if (title.empty())
                    title = "Question";
                uType |= MB_ICONQUESTION;
                break;
            }
            case MessageBoxKind::iconStop: // aka iconError
            {
                if (title.empty())
                    title = "Error";
                uType |= MB_ICONSTOP;
                break;
            }
        
        }

        MessageBoxW( 0 // hwnd
                   , fromUtf8(msg).c_str()
                   , fromUtf8(title).c_str()
                   , uType
                   );

    #else

    // !!!

    #endif

}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
enum class ProcessParentKind
{
    unknown      = -1,
    msDevenv     = 1, // MS Visual Studio
    vsCode       = 2, // VSCode
    bash         = 3, // bash/git-bash
    cmd          = 4, // cmd/git-cmd
    conEmu       = 5,
    farManager   = 6,
    explorer     = 7,

    conHost      = 127
};

//------------------------------
inline ProcessParentKind getProcessParentKind()
{
#if !(defined(WIN32) || defined(_WIN32))

    return ProcessParentKind::unknown;

#else

    if (winhelpers::isProcessHasParentOneOf({"devenv"}))
        return ProcessParentKind::msDevenv;
    else if (winhelpers::isProcessHasParentOneOf({"code"}))
        return ProcessParentKind::vsCode;
    else if (winhelpers::isProcessHasParentOneOf({"bash","git-bash"}))
        return ProcessParentKind::bash;
    else if (winhelpers::isProcessHasParentOneOf({"cmd","git-cmd"}))
        return ProcessParentKind::cmd;
    else if (winhelpers::isProcessHasParentOneOf({"ConEmuC64","ConEmu64","ConEmuC","ConEmu"}))
        return ProcessParentKind::conEmu;
    else if (winhelpers::isProcessHasParentOneOf({"far"}))
        return ProcessParentKind::farManager;
    else if (winhelpers::isProcessHasParentOneOf({"explorer"}))
        return ProcessParentKind::explorer;
    else if (winhelpers::isProcessHasParentOneOf({"conhost"}))
        return ProcessParentKind::conHost;

    return ProcessParentKind::unknown;

#endif

}


//----------------------------------------------------------------------------
namespace utils
{

inline
bool isKindOfName(const std::unordered_set<std::string>& s, const std::string &name)
{
    // return s.find(name)!=s.end();
    // return true;
    std::unordered_set<std::string>::const_iterator foundIt   = s.find(name);
    std::unordered_set<std::string>::const_iterator endIt     = s.end();
    //return foundIt!=endIt;
    return foundIt!=endIt;
}

inline
const std::unordered_set<std::string>& getBuildTypesSet()
{
    static std::unordered_set<std::string> s = { "debug", "release", "minsizerel", "relwithdebinfo" };
    return s;
}

inline
const std::unordered_set<std::string>& getOutputRootsSet()
{
    static std::unordered_set<std::string> s = { "build", "builds", "out" };
    return s;
}

inline
const std::unordered_set<std::string>& getBinFolderNamesSet()
{
    static std::unordered_set<std::string> s = { "bin" };
    return s;
}

template<typename StringType>
StringType trimFolderSpecialChars(const StringType &s)
{
    return umba::string_plus::trim_copy( s
                                       , [](auto ch)
                                         {
                                             using CharType = decltype(ch);
                                             return ch==(CharType)'.' || ch==(CharType)'_';
                                         }
                                       );
}

inline
std::string normalizeDebugAppFolderName(const std::string &name)
{
    return umba::string_plus::tolower_copy(trimFolderSpecialChars(name));
}

// Имя каталога name должно быть нормалиизовано
inline bool isKindOfBuildTypeName (const std::string &name) { return isKindOfName(getBuildTypesSet(), name); }
inline bool isKindOfOutputRootName(const std::string &name) { return isKindOfName(getOutputRootsSet(), name); }
inline bool isKindOfBinFolderName (const std::string &name) { return isKindOfName(getBinFolderNamesSet(), name); }

inline
std::string getDebugAppRootFolderGenericImplHelper(const std::vector< std::string > &pathParts, const std::string &orgPath=std::string())
{
    if (pathParts.empty())
        return orgPath;

    // Вижуалка (по умолчанию) запускает прогу с текущим каталогом выше Debug/Release пути
    // Это видимо связано с тем, что по дефолту Debug/Release кладутся прямо в корень проекта рядом с солюшеном

    if (isKindOfBuildTypeName(normalizeDebugAppFolderName(pathParts.back())))
    {
        // мы где-то в билд каталоге непосредственно
        auto idx = pathParts.size()-1;
        while(idx-->0)
        {
            if (isKindOfOutputRootName(normalizeDebugAppFolderName(pathParts[idx])))
            {
                return umba::filename::mergePath(pathParts, 0u, idx);
            }
        }

        // Ничего не смогли сделать
        return orgPath;
    }

    // Вероятно, мы выше каталога билд-конфига (Release/Debug)
    // Делаем в принципе всё то же, что и в первом варианте
    auto idx = pathParts.size();
    while(idx-->0)
    {
        if (isKindOfOutputRootName(normalizeDebugAppFolderName(pathParts[idx])))
        {
            return umba::filename::mergePath(pathParts, 0u, idx);
        }
    }

    // Ничего не смогли сделать
    return orgPath;

}


} // namespace utils


//----------------------------------------------------------------------------
inline std::string getDebugAppRootFolder( std::string *pCwd
                                        , const std::string &goDownTo=std::string() // если у нас билд каталог за пределами корня сорцов, тут можно задать, куда спуститься
                                        )
{
    std::string cwd = umba::filesys::getCurrentDirectory();

    auto cwdPathParts = umba::filename::splitPath(cwd);


    // StringType mergePath( const std::vector< StringType > &pathParts, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )

    // debug release minsizerel relwithdebinfo

    std::string rootPath;
    umba::shellapi::ProcessParentKind processParentKind = umba::shellapi::getProcessParentKind();

    if (processParentKind==umba::shellapi::ProcessParentKind::msDevenv)
    {
        // По умолчанию студия задаёт текущим каталогом на уровень выше от того, где лежит бинарник
        // Задаём c учетом принятого формата выходного каталога ".out/toolchain/platform/config"
        // ".out/toolchain/platform/..\\..\\..\\"
        // rootPath = umba::filename::makeCanonical(umba::filename::appendPath(cwd, std::string("..\\..\\..\\")));

        rootPath = utils::getDebugAppRootFolderGenericImplHelper(cwdPathParts, cwd);

    }

    else if (processParentKind==umba::shellapi::ProcessParentKind::vsCode)
    {
        // По умолчанию VSCode задаёт текущим каталогом тот, где лежит бинарник
        // Задаём c учетом принятого формата выходного каталога ".out/toolchain/platform/config"
        // ".out/toolchain/platform/config\\..\\..\\..\\..\\"

        // Не-не-не

        // rootPath = umba::filename::makeCanonical(umba::filename::appendPath(cwd, std::string("..\\..\\..\\..\\")));
        rootPath = utils::getDebugAppRootFolderGenericImplHelper(cwdPathParts, cwd);
    }

    if (!rootPath.empty())
    {
        rootPath = umba::filename::appendPathSepCopy(rootPath);
    }

    if (!goDownTo.empty())
    {
        rootPath = umba::filename::appendPathSepCopy(umba::filename::appendPath(rootPath, goDownTo));
    }

    if (pCwd)
    {
        *pCwd = cwd;
    }

    return rootPath;

}


//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)

namespace win32 {

// https://superuser.com/questions/266268/where-in-the-registry-does-windows-store-with-which-program-to-open-certain-file
// https://learn.microsoft.com/en-us/windows/win32/shell/how-to-register-a-file-type-for-a-new-application
// https://stackoverflow.com/questions/1387769/create-registry-entry-to-associate-file-extension-with-application-in-c

//----------------------------------------------------------------------------
inline HKEY getRegShellExtentionsRootHkey(bool bSystemRoot = false)
{
    if (!bSystemRoot)
       return HKEY_CURRENT_USER;
    else
       return HKEY_CLASSES_ROOT;
}

//----------------------------------------------------------------------------
inline std::wstring getRegShellExtentionHandlersRootPath(bool bSystemRoot = false)
{
    std::wstring regPath;

    if (!bSystemRoot)
    {
       regPath.append(L"Software");
       regPath.append(L"\\Classes");
    }

    return regPath;
}

//----------------------------------------------------------------------------
inline bool regSetValue(HKEY hKey, const std::wstring &varName, const std::wstring &value)
{
    LSTATUS status = RegSetValueW(hKey, varName.c_str(), REG_SZ, (LPCWSTR)value.c_str(), (DWORD)(value.size()+1)*sizeof(wchar_t));
    return status==ERROR_SUCCESS;
}

//----------------------------------------------------------------------------
// inline bool regGetValue(HKEY hKey, const std::wstring &varName, std::wstring &value)
// {
//  
// // https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
//     LSTATUS status = RegSetValueW(hKey, varName.c_str(), REG_SZ, (LPCWSTR)value.c_str(), (DWORD)(value.size()+1)*sizeof(wchar_t));
//     return status==ERROR_SUCCESS;
//  
// // LSTATUS RegGetValueW(
// //   [in]                HKEY    hkey,
// //   [in, optional]      LPCWSTR lpSubKey,
// //   [in, optional]      LPCWSTR lpValue,
// //   [in, optional]      DWORD   dwFlags,
// //   [out, optional]     LPDWORD pdwType,
// //   [out, optional]     PVOID   pvData,
// //   [in, out, optional] LPDWORD pcbData
// // );
// }

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerApplication(bool bSystemRoot, const std::wstring &appNameId, const std::wstring &shellVerb, const std::wstring &appCommand)
{

    // Компьютер\HKEY_CLASSES_ROOT\md__auto_file
    //     shell
    //       open
    //         command
    //           default value: "F:\_github\umba-tools\umba-md-pp\.out\msvc2019\x86\Debug\umba-md-pp-view.exe" "%1"
    //
    // HKEY_CLASSES_ROOT\.md_
    //     default value md__auto_file
    //
    // The nameless key is the default one - https://learn.microsoft.com/en-us/dotnet/api/microsoft.win32.registry.setvalue?view=net-8.0&redirectedfrom=MSDN#overloads

    HKEY hRootKey = getRegShellExtentionsRootHkey(bSystemRoot);

    std::wstring regPath = getRegShellExtentionHandlersRootPath(bSystemRoot);

    if (!regPath.empty())
        regPath.append(L"\\");

    regPath.append(appNameId);
    regPath.append(L"\\shell");
    regPath.append(L"\\");
    regPath.append(shellVerb);
    regPath.append(L"\\command");

    //HKEY hKey = regCreateKeyHelper(hRootKey, regPath,  /* KEY_READ| */ KEY_WRITE);
    HKEY hKey = umba::win32_utils::regCreateKey(hRootKey, regPath, KEY_WRITE);

    if (!hKey)
        return false;

    bool res = regSetValue(hKey, L"" /* varName */ , appCommand);

    RegCloseKey(hKey);

    return res;
}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerApplication(bool bSystemRoot, const std::string &appNameId, const std::string &shellVerb, const std::string &appCommand)
{
    return registerShellExtentionHandlerApplication(bSystemRoot, fromUtf8(appNameId), fromUtf8(shellVerb), fromUtf8(appCommand));

}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerForExtention(bool bSystemRoot, const std::wstring &appNameId, std::wstring ext)
{

    if (ext.empty())
        return false;

    if (ext.front()!=L'.')
    {
        ext = L"." + ext;
    }

    //HKEY hRootKey = regGetShellExtentionsRoot();
    HKEY hRootKey = getRegShellExtentionsRootHkey(bSystemRoot);

    //std::wstring regPath = regShellExtentionHandlersRootPath();
    std::wstring regPath = getRegShellExtentionHandlersRootPath(bSystemRoot);

    if (!regPath.empty())
        regPath.append(L"\\");

    regPath.append(ext);

    //HKEY hKey = regCreateKeyHelper(hRootKey, regPath,  /* KEY_READ| */ KEY_WRITE);
    HKEY hKey = umba::win32_utils::regCreateKey(hRootKey, regPath, KEY_WRITE);

    if (!hKey)
        return false;

    bool res = regSetValue(hKey, L"" /* varName */ , appNameId);

    RegCloseKey(hKey);

    return res;
}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerForExtention(bool bSystemRoot, const std::string &appNameId, const std::string &ext)
{
    return registerShellExtentionHandlerForExtention(bSystemRoot, fromUtf8(appNameId), fromUtf8(ext));
}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerForExtentionList(bool bSystemRoot, const std::wstring &appNameId, const std::vector<std::wstring> &extList)
{
    bool res = true;

    for(auto ext: extList)
    {
        if (!registerShellExtentionHandlerForExtention(bSystemRoot, appNameId, ext))
            res = false;
    }

    return res;
}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerForExtentionList(bool bSystemRoot, const std::string &appNameId, const std::vector<std::string> &extList)
{
    std::vector<std::wstring> extListW;
    for(const auto &ext: extList)
    {
        extListW.emplace_back(fromUtf8(ext));
    }

    return registerShellExtentionHandlerForExtentionList(bSystemRoot, fromUtf8(appNameId), extListW);
}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerForExtentionList(bool bSystemRoot, const std::wstring &appNameId, const std::wstring &extCommaList)
{
    //auto extList = marty_cpp::splitToLinesSimple(extCommaList, false, ',');
    auto extList = umba::string_plus::split(extCommaList, std::wstring(L",", true /* skipEmpty */ ));
    for(auto &ext: extList)
    {
        umba::string_plus::trim(ext);
    }

    return registerShellExtentionHandlerForExtentionList(bSystemRoot, appNameId, extList);
}

//----------------------------------------------------------------------------
inline bool registerShellExtentionHandlerForExtentionList(bool bSystemRoot, const std::string &appNameId, const std::string &extCommaList)
{
    return registerShellExtentionHandlerForExtentionList(bSystemRoot, fromUtf8(appNameId), fromUtf8(extCommaList));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
bool fileAttributeHiddenSet(const std::string &fname, bool bSet)
{
    auto preparedName = umba::filename::prepareForNativeUsage(umba::filesys::impl_helpers::encodeToNative(fname));
    DWORD attrs = GetFileAttributesW(preparedName.c_str());
    if (attrs==INVALID_FILE_ATTRIBUTES)
        return false;

    if (bSet)
        attrs |=  FILE_ATTRIBUTE_HIDDEN;
    else
        attrs &= ~FILE_ATTRIBUTE_HIDDEN;

    return SetFileAttributesW(preparedName.c_str(), attrs) ? true : false;
}

//----------------------------------------------------------------------------
inline
bool fileAttributeHiddenGet(const std::string &fname)
{
    auto preparedName = umba::filename::prepareForNativeUsage(umba::filesys::impl_helpers::encodeToNative(fname));
    DWORD attrs = GetFileAttributesW(preparedName.c_str());
    if (attrs==INVALID_FILE_ATTRIBUTES)
        return false;

    return (attrs&INVALID_FILE_ATTRIBUTES) ? true : false;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
bool shellParamShowHiddenFilesSet(bool bShow)
{
    SHELLSTATE  shSt = {};
    SHGetSetSettings(&shSt, SSF_SHOWALLOBJECTS, FALSE);
    shSt.fShowAllObjects = bShow ? TRUE : FALSE;
    SHGetSetSettings(&shSt, SSF_SHOWALLOBJECTS, TRUE);
    return true;
    // fShowAllObjects/SSF_SHOWALLOBJECTS   - TRUE to show all objects, including hidden files and folders. FALSE to hide hidden files and folders.
    // fShowSuperHidden/SSF_SHOWSUPERHIDDEN - TRUE to show operating system files - 
    // fShowSysFiles/SSF_SHOWSYSFILES       - TRUE to show system files, FALSE to hide them.

}

//----------------------------------------------------------------------------
inline
bool shellParamShowHiddenFilesGet(bool bShow)
{
    UMBA_USED(bShow);
    SHELLSTATE  shSt = {};
    SHGetSetSettings(&shSt, SSF_SHOWALLOBJECTS, FALSE);
    return shSt.fShowAllObjects ? true : false;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace win32

#endif

//----------------------------------------------------------------------------




} // namespace shellapi
} // namespace umba

