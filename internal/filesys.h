/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Прослойка абстракций для работы с файловой системой

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
#include "../umba.h"
//
#include "../debug_helpers.h"
//
#include "../alloca.h"
#include "../stl.h"

//
#include "env.h"
//
#include "filesys_impl_helpers.h"

//
#include <cstddef>
#include <ctime>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iterator>

#include <sys/stat.h>
#include <sys/types.h>


#if defined(WIN32) || defined(_WIN32)

    #include <Shlobj.h>

#else

    #include <errno.h>
    #include <pwd.h>
    #include <stdio.h>
    #include <unistd.h>

    #include <sys/types.h>

#endif


// umba::filename::
namespace umba{
namespace filename{

template<typename StringType> StringType getPath( const StringType &s );
std::string  getPath( const char    *p );
std::wstring getPath( const wchar_t *p );

template<typename StringType> StringType prepareForNativeUsage( const StringType &fileName );


} // namespace filename
} // namespace umba




// umba::filesys::
namespace umba{
//! Прослойка абстракций для работы с файловой системой
namespace filesys{
namespace internal{
} // namespace internal
} // namespace filesys
} // namespace umba
//----------------------------------------------------------------------------



namespace umba{
namespace filename{

#if !defined(UMBA_FILENAME_GET_NATIVE_PATH_SEP_DECLARED)
#define UMBA_FILENAME_GET_NATIVE_PATH_SEP_DECLARED
    template<typename CharType> CharType getNativePathSep( );
    template<typename StringType> StringType getNativeCurrentDirAlias( );
    template<typename StringType> StringType getNativeParentDirAlias( ) ;
    template<typename StringType> StringType getNativeHomeDirAlias( )   ;
#endif

#if !defined(UMBA_FILENAME_MAKE_CANONICAL_DECLARED)
#define UMBA_FILENAME_MAKE_CANONICAL_DECLARED
    template<typename StringType>
    StringType makeCanonical( StringType fileName
                            , typename StringType::value_type pathSep  = umba::filename::getNativePathSep<typename StringType::value_type>()
                            , const StringType &currentDirAlias        = umba::filename::getNativeCurrentDirAlias<StringType>()
                            , const StringType &parentDirAlias         = umba::filename::getNativeParentDirAlias<StringType>()
                            , bool keepLeadingParents                  = false
                            );
#endif

template<typename StringType> StringType appendPath( const StringType &p, const StringType &f, typename StringType::value_type pathSep );
template<typename StringType> StringType appendPath( const StringType &p, const StringType &f );

template<typename StringType> inline  StringType getNativeCurrentDirAlias( );
template<typename StringType> inline  StringType getNativeParentDirAlias( );


} // namespace umba
} // namespace filename
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
namespace umba
{
namespace filesys
{
namespace internal{

/*! \ingroup UMBA_CORE
 *  \addtogroup UMBA_FILESYSTEM
 *  @{
 */

using namespace umba::env::internal;

//----------------------------------------------------------------------------
//! Тип файла
enum class FileType
{
    FileTypeInvalid, //!< Файл не найден, или ещё какая-то проблема
    FileTypeUnknown, //!< Тип файла не известен
    FileTypeDir    , //!< Каталог, не файл
    FileTypeFile   , //!< Файл
    FileTypeRam      //!< Что-то просто лежит в памяти
};

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename DataType> inline
bool readFile(std::istream &fileIn, std::vector<DataType> &filedata)
{
    filedata.clear();

    DataType buf[4096];
    while (fileIn.read(buf, sizeof(buf)))
    {
        // Except in the constructors of std::strstreambuf, negative values of std::streamsize are never used.
        std::size_t readedBytes = (size_t)fileIn.gcount();
        std::size_t readedItems = readedBytes/sizeof(DataType);
        filedata.insert(filedata.end(), &buf[0], &buf[readedItems]);
    }

    std::size_t readedBytes = (size_t)fileIn.gcount();
    std::size_t readedItems = readedBytes/sizeof(DataType);
    filedata.insert(filedata.end(), &buf[0], &buf[readedItems]);

    return true;
}

inline
bool readFile(std::istream &fileIn, std::string &filedata)
{
    filedata.clear();

    char buf[4096];
    while (fileIn.read(buf, sizeof(buf)))
    {
        // Except in the constructors of std::strstreambuf, negative values of std::streamsize are never used.
        std::size_t readedBytes = (std::size_t)fileIn.gcount();
        std::size_t readedItems = readedBytes/sizeof(char);
        filedata.append(&buf[0], readedItems);
    }

    std::size_t readedBytes = (std::size_t)fileIn.gcount();
    std::size_t readedItems = readedBytes/sizeof(char);
    filedata.append(&buf[0], readedItems);

    return true;
}

//----------------------------------------------------------------------------
template<typename DataType> inline
bool writeFile(std::ostream &fileOut, const DataType *pData, size_t dataSize)
{
    if (!dataSize)
         return true;

    const size_t itemSize = sizeof(DataType);
    const size_t numRawBytesToWrite = dataSize*itemSize;

    if (!fileOut.write(pData, (std::streamsize)numRawBytesToWrite))
    {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
template<typename DataType> inline
bool writeFile(std::ostream &fileOut, const std::vector<DataType> &data)
{
    auto dataSize = data.size();
    if (!dataSize)
        return true;

    if (data.empty())
    {
        DataType d;
        return writeFile(fileOut, &d, 0);
    }
    else
    {
        return writeFile(fileOut, &data[0], data.size());
    }

}

//----------------------------------------------------------------------------
inline
bool writeFile(std::ostream &fileOut, const std::string &data)
{
    return writeFile(fileOut, data.data(), data.size());
}


//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)
    typedef   __time64_t  filetime_t; //!< Файловое время - время создания, изменения, доступа, и тп
    typedef   uint64_t    filesize_t; //!< Тип для хранения размера файла
#else
    typedef   time_t      filetime_t; //!< Файловое время - время создания, изменения, доступа, и тп. Для *nix/MCU используется дефолтный тип, который ограничен 2038 годом. Я уже буду одной ногой на пенсии
    typedef   uint64_t    filesize_t; //!< Тип для хранения размера файла
#endif

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Шаблонная функция форматирования файлового даты/времени (C++ аналог strftime). Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
template<typename StringType> inline
StringType formatFiletime( filetime_t t, const StringType &fmt );

//------------------------------

#if defined(WIN32) || defined(_WIN32)

//------------------------------
//! Хелпер конвертации юниксового filetime_t в виндовый
inline FILETIME convertUnixTimeT( filetime_t t )
{
    filetime_t t1 = t, t2 = 10000000ull, t3 = 116444736000000000ull;
    LONGLONG ll = t1*t2 + t3;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD) ll;
    ft.dwHighDateTime = (DWORD)(ll >>32);
    return ft;
}

//------------------------------
//! Хелпер конвертации виндового FILETIME в юниксовый
inline filetime_t convertWindowsFiletime( FILETIME ft )
{
    filetime_t ftH = ft.dwHighDateTime;
    filetime_t ftL = ft.dwLowDateTime;
    //filetime_t ft

    filetime_t wndFt = (ftH<<32)+ftL;

    return (filetime_t)((wndFt - 116444736000000000ull) / 10000000ull);
}

//------------------------------
//! Версия std::string strftime для C++. Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
template<> inline
std::string formatFiletime<std::string>( filetime_t t, const std::string &fmt )
{
    if (fmt.empty())
        return fmt;

    std::tm tmBuf;
    if (_localtime64_s(&tmBuf, &t)!=0)
        return std::string("INVALID_TIME");

    char buf[64];
    size_t zpos = strftime( &buf[0], 63, fmt.c_str(), &tmBuf );
    if (!zpos)
        return std::string("TOO_LONG_TIME_FORMAT");

    buf[zpos] = 0;

    return std::string(buf);
}

//------------------------------
//! Версия std::wstring strftime для C++. Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
template<> inline
std::wstring formatFiletime<std::wstring>( filetime_t t, const std::wstring &fmt )
{
    if (fmt.empty())
        return fmt;

    std::tm tmBuf;
    if (_localtime64_s(&tmBuf, &t)!=0)
        return std::wstring(L"INVALID_TIME");

    wchar_t buf[64];
    size_t zpos = wcsftime( &buf[0], 63, fmt.c_str(), &tmBuf );
    if (!zpos)
        return std::wstring(L"TOO_LONG_TIME_FORMAT");

    buf[zpos] = 0;

    return std::wstring(buf);
}

//------------------------------

#else

//------------------------------
//! Версия std::string strftime для C++. Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
template<> inline
std::string formatFiletime<std::string>( filetime_t t, const std::string &fmt )
{
    if (fmt.empty())
        return fmt;

    std::tm *ptmBuf = std::localtime(&t);
    char buf[64];
    size_t zpos = strftime( &buf[0], 63, fmt.c_str(), &tmBuf );
    if (!zpos)
        return std::string("TOO_LONG_TIME_FORMAT");

    buf[zpos] = 0;

    return std::string(buf);
}

//------------------------------
//! Версия std::wstring strftime для C++. Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
template<> inline
std::wstring formatFiletime<std::wstring>( filetime_t t, const std::wstring &fmt )
{
    return umba::string_plus::make_string<std::wstring>( formatFiletime<std::string>( t, umba::string_plus::make_string<std::string>(fmt) ) );
}

//------------------------------

#endif

//------------------------------


//------------------------------
//! Версия strftime для C++, принимающая указатель на сырую форматную строку. Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
template<typename StringType> inline
StringType formatFiletime( filetime_t t, const typename StringType::value_type *fmt )
{
    return formatFiletime<StringType>( t, StringType(fmt) );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Данные статистики по файлу
#include "../warnings/push_disable_padding_added.h"
struct FileStat
{
    FileType     fileType;         //!< Тип файла - файл, каталог, хз, ошибка
    filesize_t   fileSize;         //!< Размер файла
    filetime_t   timeCreation    ; //!< Время создания
    filetime_t   timeLastModified; //!< Время последней модификации
    filetime_t   timeLastAccess  ; //!< Время последнего доступа

    //! Возвращает true, если тип файла валидный
    bool isValid() const { return fileType!=FileType::FileTypeInvalid; }

    //! Возвращает true, если статистика - по каталогу
    bool isDir()   const { return fileType==FileType::FileTypeDir    ; }

    //! Возвращает true, если статистика - по файлу
    bool isFile()  const { return fileType==FileType::FileTypeFile   ; }
};
#include "../warnings/pop.h"
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)

    //! Одно универсальное имя для struct stat
    #define struct_file_stat               struct __stat64

    //! Хелпер получения статистики файла в ОС-зависимом формате по имени в std::string
    inline int struct_file_stat_get_stat( const std::string &fileName, struct_file_stat *statBuf )
    {
        return _stat64( fileName.c_str(), statBuf );
    }

    //! Хелпер получения статистики файла в ОС-зависимом формате по имени в std::wstring
    inline int struct_file_stat_get_stat( const std::wstring &fileName, struct_file_stat *statBuf )
    {
        return _wstat64( fileName.c_str(), statBuf );
    }


    inline
    FileStat fileStatFromFindData(const WIN32_FIND_DATAW &findData)
    {
        FileStat fileStat;

        if (findData.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY)) // |FILE_ATTRIBUTE_REPARSE_POINT - репарс поинт может быть как файлом, так и каталогом, 
            fileStat.fileType = FileType::FileTypeDir;
        else
            fileStat.fileType = FileType::FileTypeFile;

// if(result->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
// {
//     if(result->dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT)
//     {
//         //path is a volume; try using GetVolumeNameForVolumeMountPoint for info
//     }
// }

        //fileStat.fileType         = (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? FileType::FileTypeDir : FileType::FileTypeFile;
        fileStat.fileSize         = (((filesize_t)findData.nFileSizeHigh) << 32) | ((filesize_t)findData.nFileSizeLow);

        fileStat.timeCreation     = convertWindowsFiletime(findData.ftCreationTime);
        fileStat.timeLastModified = convertWindowsFiletime(findData.ftLastWriteTime);
        fileStat.timeLastAccess   = convertWindowsFiletime(findData.ftLastAccessTime);

        return fileStat;
    }

    inline
    FileStat fileStatFromFindData(const WIN32_FIND_DATAA &findData)
    {
        FileStat fileStat;

        if (findData.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY)) // |FILE_ATTRIBUTE_REPARSE_POINT
            fileStat.fileType = FileType::FileTypeDir;
        else
            fileStat.fileType = FileType::FileTypeFile;
        //fileStat.fileType         = (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? FileType::FileTypeDir : FileType::FileTypeFile;
        fileStat.fileSize         = (((filesize_t)findData.nFileSizeHigh) << 32) | ((filesize_t)findData.nFileSizeLow);

        fileStat.timeCreation     = convertWindowsFiletime(findData.ftCreationTime);
        fileStat.timeLastModified = convertWindowsFiletime(findData.ftLastWriteTime);
        fileStat.timeLastAccess   = convertWindowsFiletime(findData.ftLastAccessTime);

        return fileStat;
    }


#else

    //! Одно универсальное имя для struct stat
    #define struct_file_stat    struct stat

    //! Хелпер получения статистики файла в ОС-зависимом формате по имени в std::string
    inline int struct_file_stat_get_stat( const std::string &fileName, struct_file_stat *statBuf )
    {
        return stat( fileName.c_str(), statBuf );
    }

    //! Хелпер получения статистики файла в ОС-зависимом формате по имени в std::wstring
    inline int struct_file_stat_get_stat( const std::wstring &fileName, struct_file_stat *statBuf )
    {
        #ifdef UMBA_DEBUGBREAK
            UMBA_DEBUGBREAK();
        #endif
        throw std::runtime_error("Not implemented: wide version of the struct_file_stat_get_stat (getFileStat) not implemented for non-WIN32");
    }

#endif

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
//! Хелпер конвертации системной статистики в нашу
inline
void parseStatToFileStat( const struct_file_stat &statBuf, FileStat &fileStat )
{
    fileStat.fileSize         = (filesize_t)statBuf.st_size;
    fileStat.timeCreation     =             statBuf.st_ctime;
    fileStat.timeLastModified =             statBuf.st_mtime;
    fileStat.timeLastAccess   =             statBuf.st_atime;

    fileStat.fileType = FileType::FileTypeUnknown;

    #if defined(WIN32) || defined(_WIN32)
    if (statBuf.st_mode&_S_IFDIR)
        fileStat.fileType = FileType::FileTypeDir;
    else if (statBuf.st_mode&_S_IFREG)
        fileStat.fileType = FileType::FileTypeFile;
    #else
    if (statBuf.st_mode&S_IFDIR)
        fileStat.fileType = FileTypeDir;
    else if (statBuf.st_mode&S_IFREG)
        fileStat.fileType = FileTypeFile;
    #endif
}

//----------------------------------------------------------------------------
//! Возвращает FileStat по пути (файл или каталог, не важно)
template<typename StringType>
bool getPathStat(const StringType &path, FileStat &fileStat)
{
    UMBA_USED(path);
    UMBA_USED(fileStat);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: getPathStat");
}

//----------------------------------------------------------------------------
template<typename StringType>
bool isPathExist(const StringType &path)
{
    FileStat fileStat;
    return getPathStat(path, fileStat);
}

//----------------------------------------------------------------------------
template<typename StringType>
bool isPathDirectory(const StringType &path)
{
    FileStat fileStat;
    if (!getPathStat(path, fileStat))
        return false;

    return fileStat.isDir();
}

//----------------------------------------------------------------------------
template<typename StringType>
bool isPathFile(const StringType &path)
{
    FileStat fileStat;
    if (!getPathStat(path, fileStat))
        return false;

    return fileStat.isFile();
}

//----------------------------------------------------------------------------
template<typename StringType>
bool isFileExist(const StringType &path)
{
    FileStat fileStat;
    if (!getPathStat(path, fileStat))
        return false;
    return fileStat.isFile();
}

//----------------------------------------------------------------------------
template<typename StringType>
bool isDirExist(const StringType &path)
{
    FileStat fileStat;
    if (!getPathStat(path, fileStat))
        return false;
    return fileStat.isDir();
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)

//TODO: !!! Посмотреть "Именование файлов, путей и пространств имен" - https://learn.microsoft.com/ru-ru/windows/win32/fileio/naming-a-file

//----------------------------------------------------------------------------
template<> inline
bool getPathStat<std::string>(const std::string &path, FileStat &fileStat)
{
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(path.c_str(), &findData);
    if (hFind==INVALID_HANDLE_VALUE)
        return false;

    fileStat = fileStatFromFindData(findData);

    FindClose(hFind);

    return true;
}

//----------------------------------------------------------------------------
template<> inline
bool getPathStat<std::wstring>(const std::wstring &path, FileStat &fileStat)
{
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.c_str(), &findData);
    if (hFind==INVALID_HANDLE_VALUE)
        return false;

    fileStat = fileStatFromFindData(findData);

    FindClose(hFind);

    return true;
}

//----------------------------------------------------------------------------
//! Хелпер (std::string) для генерик открытия файла для чтения
inline
HANDLE openFileForReadingWin32(const std::string &filename)
{
    if (filename.empty()) return INVALID_HANDLE_VALUE;

    return CreateFileA( filename.c_str(), GENERIC_READ
                      , FILE_SHARE_READ | FILE_SHARE_WRITE
                      , 0 // lpSecurityAttributes
                      , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL
                      , 0 // hTemplateFile
                      );
}

//! Хелпер (std::wstring) для генерик открытия файла для чтения
inline
HANDLE openFileForReadingWin32(const std::wstring &filename)
{
    if (filename.empty()) return INVALID_HANDLE_VALUE;

    return CreateFileW( umba::filename::prepareForNativeUsage(filename).c_str(), GENERIC_READ
                      , FILE_SHARE_READ | FILE_SHARE_WRITE
                      , 0 // lpSecurityAttributes
                      , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL
                      , 0 // hTemplateFile
                      );
}

//----------------------------------------------------------------------------
//! Хелпер (std::string) для генерик открытия файла для записи
inline
HANDLE openFileForWrittingWin32(const std::string &filename, bool bOverwrite)
{
    if (filename.empty()) return INVALID_HANDLE_VALUE;

    DWORD dwCreationDisposition = bOverwrite
                                ? (DWORD)CREATE_ALWAYS // Creates a new file, always. If the specified file exists and is writable, the function overwrites the file
                                : (DWORD)CREATE_NEW    // Creates a new file, only if it does not already exist. If the specified file exists, the function fails
                                ;

    return CreateFileA( umba::filename::prepareForNativeUsage(filename).c_str(), GENERIC_WRITE
                      , FILE_SHARE_READ
                      , 0 // lpSecurityAttributes
                      , dwCreationDisposition, FILE_ATTRIBUTE_NORMAL
                      , 0 // hTemplateFile
                      );
}

//! Хелпер (std::wstring) для генерик открытия файла для записи
inline
HANDLE openFileForWrittingWin32(const std::wstring &filename, bool bOverwrite)
{
    if (filename.empty()) return INVALID_HANDLE_VALUE;

    DWORD dwCreationDisposition = bOverwrite
                                ? (DWORD)CREATE_ALWAYS // Creates a new file, always. If the specified file exists and is writable, the function overwrites the file
                                : (DWORD)CREATE_NEW    // Creates a new file, only if it does not already exist. If the specified file exists, the function fails
                                ;

    return CreateFileW( umba::filename::prepareForNativeUsage(filename).c_str(), GENERIC_WRITE
                      , FILE_SHARE_READ
                      , 0 // lpSecurityAttributes
                      , dwCreationDisposition, FILE_ATTRIBUTE_NORMAL
                      , 0 // hTemplateFile
                      );
}

//----------------------------------------------------------------------------
inline
HANDLE openFileExistingExclusiveForReadingWrittingWin32(const std::string &filename)
{
    if (filename.empty()) return INVALID_HANDLE_VALUE;

    DWORD dwCreationDisposition = OPEN_EXISTING; // Opens a file or device, only if it exists. If the specified file or device does not exist, the function fails and the last-error code is set to ERROR_FILE_NOT_FOUND (2).

    return CreateFileA( umba::filename::prepareForNativeUsage(filename).c_str(), GENERIC_READ | GENERIC_WRITE
                      , 0 // Prevents other processes from opening a file or device if they request delete, read, or write access.
                      , 0 // lpSecurityAttributes
                      , dwCreationDisposition, FILE_ATTRIBUTE_NORMAL
                      , 0 // hTemplateFile
                      );
}

inline
HANDLE openFileExistingExclusiveForReadingWrittingWin32(const std::wstring &filename)
{
    if (filename.empty()) return INVALID_HANDLE_VALUE;

    DWORD dwCreationDisposition = OPEN_EXISTING; // Opens a file or device, only if it exists. If the specified file or device does not exist, the function fails and the last-error code is set to ERROR_FILE_NOT_FOUND (2).

    return CreateFileW( umba::filename::prepareForNativeUsage(filename).c_str(), GENERIC_READ | GENERIC_WRITE
                      , 0 // Prevents other processes from opening a file or device if they request delete, read, or write access.
                      , 0 // lpSecurityAttributes
                      , dwCreationDisposition, FILE_ATTRIBUTE_NORMAL
                      , 0 // hTemplateFile
                      );
}

//----------------------------------------------------------------------------



// https://docs.microsoft.com/en-us/windows/win32/sysinfo/converting-a-time-t-value-to-a-file-time

//! Хелпер получения статистики файла
template<typename StringType> inline
FileStat getFileStat( const StringType &fileName )
{

    // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfiletime

    // https://docs.microsoft.com/en-us/windows/win32/sysinfo/converting-a-time-t-value-to-a-file-time

    FileStat fileStat;
    fileStat.fileType = FileType::FileTypeInvalid;


    HANDLE hFile = openFileForReadingWin32( fileName.c_str() );
    if (hFile==INVALID_HANDLE_VALUE)
    {
        // DWORD err = GetLastError();
        return fileStat;
    }


    FILETIME creationTime;
    FILETIME lastAccessTime;
    FILETIME lastWriteTime;

    if (!GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime))
    {
        CloseHandle(hFile);
        return fileStat;
    }

    LARGE_INTEGER liFileSize;
    if (!GetFileSizeEx(hFile,&liFileSize))
    {
        CloseHandle(hFile);
        return fileStat;
    }

    fileStat.fileSize = (filesize_t)liFileSize.QuadPart;


    fileStat.fileType = FileType::FileTypeFile; // Need to check is dir
    // enum FileType { FileTypeInvalid, FileTypeUnknown, FileTypeDir, FileTypeFile, FileTypeRam };

    fileStat.timeCreation     = convertWindowsFiletime(creationTime);
    fileStat.timeLastModified = convertWindowsFiletime(lastAccessTime);
    fileStat.timeLastAccess   = convertWindowsFiletime(lastWriteTime);

    CloseHandle(hFile);
    return fileStat;
}

#else // !WIN32

//! Хелпер получения статистики файла
template<typename StringType> inline
FileStat getFileStat( const StringType &fileName )
{
    FileStat fileStat;
    fileStat.fileType = FileTypeInvalid;

    if (fileName.empty())
        return fileStat;

    struct_file_stat buffer;
    int res = struct_file_stat_get_stat( fileName, &buffer );

    if (res<0)
        return fileStat;

    parseStatToFileStat( buffer, fileStat );
    return fileStat;
}


#endif

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Получение filetime_t на текущий момент
inline filetime_t getFileTimeNow()
{
    #if defined(WIN32) || defined(_WIN32)
    __time64_t t;
    _time64(&t);
    #else
    time_t t;
    time(&t);
    #endif

    return (filetime_t)t;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
/*
    Нужно а) уметь проверять файл по имени на доступность для чтения
          б) сразу его читать без переоткрытия

          Или фик с ним, и не париться с преждевременной оптимизацией?
          По идее, два последовательных открытия не должны быть сильно узким местом

    https://docs.microsoft.com/en-us/windows/win32/fileio/creating-and-opening-files
    https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea

*/
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Получение текущего рабочего каталога - генерик версия
template<typename StringType> inline
StringType getCurrentDirectory()
{
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: getCurrentDirectory not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Получение текущего рабочего каталога - генерик версия
template<typename StringType> inline
bool setCurrentDirectory(const StringType &newCurDir)
{
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: setCurrentDirectory not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Перемещение (переименование) файла или каталога
template<typename StringType> inline
bool moveFileOrDirectory(const StringType &oldName, const StringType &newName)
{
    UMBA_USED(oldName);
    UMBA_USED(newName);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: moveFileOrDirectory not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Копирование файла
template<typename StringType> inline
bool copyFile(const StringType &oldName, const StringType &newName, bool bOverwrite)
{
    UMBA_USED(oldName);
    UMBA_USED(newName);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: copyFile not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Удаление файла
template<typename StringType> inline
bool deleteFile( const StringType &filename )
{
    UMBA_USED(filename);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: deleteFile not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Удаление каталога
template<typename StringType> inline
bool deleteDirectory( const StringType &dirname )
{
    UMBA_USED(dirname);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: deleteDirectory not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Создание каталога
template<typename StringType> inline bool createDirectory( const StringType &dirname )
{
    UMBA_USED(dirname);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: createDirectory not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Создание каталога
template<typename StringType> inline bool createDirectoryEx( const StringType &dirname, bool forceCreatePath )
{
    UMBA_USED(dirname);
    UMBA_USED(forceCreatePath);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: createDirectory not specialized for this StringType");
}

// //----------------------------------------------------------------------------
// //! Проверка, является ли путь каталогом
// template<typename StringType> inline bool isPathDirectory( const StringType &path )
// {
//     UMBA_USED(path);
//     throw std::runtime_error("Not implemented: isPathDirectory not specialized for this StringType");
// }

//----------------------------------------------------------------------------
//! Проверка, является ли файл файлом только для чтения
template<typename StringType> inline bool isFileReadonly( const StringType &fname )
{
    UMBA_USED(fname);
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: isFileReadonly not specialized for this StringType");
}

//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)
inline
bool isLastErrorAlreadyExists()
{
    return ::GetLastError()==ERROR_ALREADY_EXISTS;
}
#else
inline
bool isLastErrorAlreadyExists()
{
    return errno==EEXIST;
}
#endif

//----------------------------------------------------------------------------
template<typename StringType>
bool isFileExistingExclusiveReadableWrittable(const StringType &fname)
{
#if defined(WIN32) || defined(_WIN32)

    HANDLE hFile = openFileExistingExclusiveForReadingWrittingWin32(fname);
    if (hFile==INVALID_HANDLE_VALUE) return false;
    CloseHandle(hFile);
    return true;

#else

    // !!! На не винде проверяем доступность на запись и чтение через плюсовые потоки
    {
        std::ofstream ofs;
        ofs.clear();
        ofs.open( fname.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::ate ); // noreplace (C++23) open in exclusive mode
        if (!ofs)
            return false;
    }

    {
        std::ifstream ifs;
        ifs.clear();
        ifs.open( fname.c_str(), std::ios_base::binary | std::ios_base::in ); // noreplace (C++23) open in exclusive mode
        if (!ifs)
            return false;
    }

    return true;

#endif
}



//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)

//----------------------------------------------------------------------------
//! Перемещение (переименование) файла или каталога, специализация для std::string
template<> inline
bool moveFileOrDirectory<std::string>(const std::string &oldName, const std::string &newName)
{
    return ::MoveFileA( umba::filename::prepareForNativeUsage(oldName).c_str() // lpExistingFileName
                      , umba::filename::prepareForNativeUsage(newName).c_str() // lpNewFileName
                      ) ? true : false;
}

//----------------------------------------------------------------------------
//! Перемещение (переименование) файла или каталога, специализация для std::wstring
template<> inline
bool moveFileOrDirectory<std::wstring>(const std::wstring &oldName, const std::wstring &newName)
{
    return ::MoveFileW( umba::filename::prepareForNativeUsage(oldName).c_str() // lpExistingFileName
                      , umba::filename::prepareForNativeUsage(newName).c_str() // lpNewFileName
                      ) ? true : false;
}

//----------------------------------------------------------------------------
//! Копирование файла, специализация для std::string
template<> inline
bool copyFile<std::string>(const std::string &oldName, const std::string &newName, bool bOverwrite)
{
    return ::CopyFileA( umba::filename::prepareForNativeUsage(oldName).c_str() // lpExistingFileName
                      , umba::filename::prepareForNativeUsage(newName).c_str() // lpNewFileName
                      , bOverwrite ? FALSE /* !bFailIfExists */ : TRUE /* bFailIfExists */
                      ) ? true : false;
}

//----------------------------------------------------------------------------
//! Копирование файла, специализация для std::wstring
template<> inline
bool copyFile<std::wstring>(const std::wstring &oldName, const std::wstring &newName, bool bOverwrite)
{
    return ::CopyFileW( umba::filename::prepareForNativeUsage(oldName).c_str() // lpExistingFileName
                      , umba::filename::prepareForNativeUsage(newName).c_str() // lpNewFileName
                      , bOverwrite ? FALSE /* !bFailIfExists */ : TRUE /* bFailIfExists */
                      ) ? true : false;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Удаление файла, специализация для std::string
template<> inline
bool deleteFile<std::string>( const std::string &filename )
{
    return ::DeleteFileA(umba::filename::prepareForNativeUsage(filename).c_str()) ? true : false;
}

//----------------------------------------------------------------------------
//! Удаление файла, специализация для std::wstring
template<> inline
bool deleteFile<std::wstring>( const std::wstring &filename )
{
    return ::DeleteFileW(umba::filename::prepareForNativeUsage(filename).c_str()) ? true : false;
}

//----------------------------------------------------------------------------
//! Удаление каталога, специализация для std::string
template<> inline
bool deleteDirectory<std::string>( const std::string &filename )
{
    return ::RemoveDirectoryA(umba::filename::prepareForNativeUsage(filename).c_str()) ? true : false;
}

//----------------------------------------------------------------------------
//! Удаление каталога, специализация для std::wstring
template<> inline
bool deleteDirectory<std::wstring>( const std::wstring &filename )
{
    return ::RemoveDirectoryW(umba::filename::prepareForNativeUsage(filename).c_str()) ? true : false;
}

//----------------------------------------------------------------------------
/*
template<typename StringType> inline bool createDirectory( const StringType &dirname )
*/
//! Создание каталога, специализация для std::string
template<> inline bool createDirectory<std::string>( const std::string &dirname )
{
    bool bRes = ::CreateDirectoryA(umba::filename::prepareForNativeUsage(dirname).c_str(), 0)==0 ? false : true;
    if (bRes)
    {
        DWORD dwAttrs = GetFileAttributesA(umba::filename::prepareForNativeUsage(dirname).c_str());
        if (dwAttrs!=INVALID_FILE_ATTRIBUTES)
        {
            SetFileAttributesA(umba::filename::prepareForNativeUsage(dirname).c_str(), dwAttrs|FILE_ATTRIBUTE_DIRECTORY);
        }
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
//! Создание каталога, специализация для std::wstring
template<> inline bool createDirectory<std::wstring>( const std::wstring &dirname )
{
    bool bRes = ::CreateDirectoryW(umba::filename::prepareForNativeUsage(dirname).c_str(), 0)==0 ? false : true;
    if (bRes)
    {
        DWORD dwAttrs = GetFileAttributesW(umba::filename::prepareForNativeUsage(dirname).c_str());
        if (dwAttrs!=INVALID_FILE_ATTRIBUTES)
        {
            SetFileAttributesW(umba::filename::prepareForNativeUsage(dirname).c_str(), dwAttrs|FILE_ATTRIBUTE_DIRECTORY);
        }
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
//! Создание каталога, специализация для std::string
template<> inline bool createDirectoryEx<std::string>( const std::string &dirname, bool forceCreatePath )
{
    if (!forceCreatePath)
    {
        return createDirectory(dirname);
    }

    std::vector<std::string> pathList;

    std::string path = dirname;

    while(!path.empty() && !createDirectory(path) && GetLastError()!=ERROR_ALREADY_EXISTS)
    {
        pathList.emplace_back(path);
        auto newPath = umba::filename::getPath(path);
        if (newPath==path)
        {
            break;
        }

        path = newPath;
    }

    std::vector<std::string>::const_reverse_iterator it = pathList.rbegin();
    for(; it!=pathList.rend(); ++it)
    {
        // if (it==pathList.rbegin() && !it->empty() && it->back()==':')
        // {
        //     continue;
        // }

        if (!createDirectory(*it))
        {
            auto err = GetLastError();
            if (err==ERROR_ALREADY_EXISTS) // 183L
            {
                continue;
            }

            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------
//! Создание каталога, специализация для std::wstring
template<> inline bool createDirectoryEx<std::wstring>( const std::wstring &dirname, bool forceCreatePath )
{
    if (!forceCreatePath)
    {
        return createDirectory(dirname);
    }

    std::vector<std::wstring> pathList;

    std::wstring path = dirname;

    while(!path.empty() && !createDirectory(path) && GetLastError()!=ERROR_ALREADY_EXISTS)
    {
        pathList.emplace_back(path);
        auto newPath = umba::filename::getPath(path);
        if (newPath==path)
        {
            break;
        }

        path = newPath;
    }

    std::vector<std::wstring>::const_reverse_iterator it = pathList.rbegin();
    for(; it!=pathList.rend(); ++it)
    {
        // if (it==pathList.rbegin() && !it->empty() && it->back()==L':')
        // {
        //     continue;
        // }

        if (!createDirectory(*it))
        {
            auto err = GetLastError();
            // #define ERROR_INVALID_NAME               123L    // dderror
            if (err==ERROR_ALREADY_EXISTS) // 183L
            {
                continue;
            }

            return false;
        }
    }

    return true;
}

#if 0
// Почему-то это не работает

//----------------------------------------------------------------------------
//! Проверка, является ли путь каталогом, специализация для std::string
template<> inline bool isPathDirectory<std::string>( const std::string &path )
{
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesa?redirectedfrom=MSDN
    DWORD attrs = ::GetFileAttributesA(umba::filename::prepareForNativeUsage(path).c_str());
    if (attrs==INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    // https://learn.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
//! Проверка, является ли путь каталогом, специализация для std::wstring
template<> inline bool isPathDirectory<std::wstring>( const std::wstring &path )
{
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesa?redirectedfrom=MSDN
    DWORD attrs = ::GetFileAttributesW(umba::filename::prepareForNativeUsage(path).c_str());
    if (attrs==INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    // https://learn.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;
    }

    return false;
}
#endif

//----------------------------------------------------------------------------
//! Проверка, является ли файл файлом только для чтения, специализация для std::string
template<> inline bool isFileReadonly<std::string>( const std::string &fname )
{
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesa?redirectedfrom=MSDN
    DWORD attrs = ::GetFileAttributesA(umba::filename::prepareForNativeUsage(fname).c_str());
    if (attrs==INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    // https://learn.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }

    if (attrs & FILE_ATTRIBUTE_READONLY)
    {
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
//! Проверка, является ли файл файлом только для чтения, специализация для std::wstring
template<> inline bool isFileReadonly<std::wstring>( const std::wstring &fname )
{
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesa?redirectedfrom=MSDN
    DWORD attrs = ::GetFileAttributesW(umba::filename::prepareForNativeUsage(fname).c_str());
    if (attrs==INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    // https://learn.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }

    if (attrs & FILE_ATTRIBUTE_READONLY)
    {
        return true;
    }

    return false;
}


//----------------------------------------------------------------------------
//! Проверка доступности файла на чтение
template<typename StringType> inline
bool isFileReadable( const StringType &filename )
{
    HANDLE hFile = openFileForReadingWin32( filename.c_str() );
    if (hFile==INVALID_HANDLE_VALUE) return false;
    CloseHandle(hFile);
    return true;
}

//------------------------------
// "vector" class used for storing data cause it's allow access to raw data for writting in C++ 0x03
// In C++ 0x11 string class also has the same functionality
//----------------------------------------------------------------------------
//! Чтение файла в вектор
/*!
    \tparam StringType Тип имени файла - std::string/std::wstring
    \tparam DataType   Тип читаемых данных

    \return Возвращает true, если файл был прочитан
 */
template<typename StringType, typename DataType> inline
bool readFile( const StringType &filename       //!< Имя файла
             , std::vector<DataType> &filedata  //!< Вектор для данных
             , FileStat *pFileStat = 0          //!< [out] Статистика файла
             , bool ignoreSizeErrors = true     //!< Игнорировать разночтения из статистики файла и реально прочитанного размера
             )
{
    filedata.clear();

    FileStat fileStat = getFileStat( filename );
    if (fileStat.fileType!=FileType::FileTypeFile)
        return false;

    if (pFileStat)
        *pFileStat = fileStat;

    if (fileStat.fileSize==0)
        return true; // no data for reading at all

    if (!isFileReadable( filename ))
        return false;

    const size_t itemSize = sizeof(DataType);
    const size_t numItems = (size_t)(fileStat.fileSize / itemSize);
    filedata.resize( numItems ); // We can read files which are always can fit to memory
    const size_t numRawBytesToRead = filedata.size()*itemSize;

    // Here starts "no exceptions" (exception safe) zone

    HANDLE hFile = openFileForReadingWin32( filename.c_str() );
    if (hFile==INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        UMBA_USED(err);
        return false;
    }

    DWORD readedBytes = 0;

    BOOL bReadRes = ReadFile( hFile, &filedata[0], (DWORD)numRawBytesToRead, &readedBytes, 0 );

    CloseHandle(hFile);

    if (!bReadRes)
    {
        filedata.clear(); // Ошибка. Если вектор не временный, то неплохо бы его обнулить, чтобы место в памяти не хавал.
        return false;
    }

    if (readedBytes!=(DWORD)fileStat.fileSize)
    {
        if (ignoreSizeErrors)
        {
            filedata.resize((std::size_t)(readedBytes/numItems), (DataType)0);
            fileStat.fileSize = (filesize_t)readedBytes; // сохраняем консистенцию размера вектора и fileStat'а. Или не надо?
            if (pFileStat)
                *pFileStat = fileStat;
            return true;
        }

        // Чистим вектор, и возвращаем ошибку
        filedata.clear();
        return false;
    }

    // Вроде всё хорошоу
    return true;
}

//------------------------------
template<typename StringType> inline
bool readFile( const StringType &filename       //!< Имя файла
             , std::string      &filedata       //!< Строка для данных
             , FileStat *pFileStat = 0          //!< [out] Статистика файла
             , bool ignoreSizeErrors = true     //!< Игнорировать разночтения из статистики файла и реально прочитанного размера
             )
{
    #ifdef UMBA_CXX_HAS_STD17

        filedata.clear();

        FileStat fileStat = getFileStat( filename );
        if (fileStat.fileType!=FileType::FileTypeFile)
            return false;

        if (pFileStat)
            *pFileStat = fileStat;

        if (fileStat.fileSize==0)
            return true; // no data for reading at all

        if (!isFileReadable( filename ))
            return false;

        const size_t itemSize = sizeof(char);
        const size_t numItems = (size_t)(fileStat.fileSize / itemSize);
        {
            std::string strTmp = std::string(numItems, (char)0);
            std::swap(strTmp, filedata);
            // filedata.resize( numItems ); // We can read files which are always can fit to memory
        }

        // const size_t numRawBytesToRead = filedata.size()*itemSize;

        // Here starts "no exceptions" (exception safe) zone

        HANDLE hFile = openFileForReadingWin32( filename.c_str() );
        if (hFile==INVALID_HANDLE_VALUE)
        {
            // DWORD err = GetLastError();
            return false;
        }

        DWORD readedBytes = 0;

        BOOL bReadRes = ReadFile( hFile, filedata.data(), (DWORD)filedata.size(), &readedBytes, 0 );

        CloseHandle(hFile);

        if (!bReadRes)
        {
            filedata.clear(); // Ошибка. Если вектор не временный, то неплохо бы его обнулить, чтобы место в памяти не хавал.
            return false;
        }

        if (readedBytes!=(DWORD)fileStat.fileSize)
        {
            if (ignoreSizeErrors)
            {
                filedata.resize((std::size_t)readedBytes, (char)0);
                fileStat.fileSize = (filesize_t)filedata.size(); // сохраняем консистенцию размера вектора и fileStat'а. Или не надо?
                if (pFileStat)
                    *pFileStat = fileStat;
                return true;
            }

            // Чистим вектор, и возвращаем ошибку
            filedata.clear();
            return false;
        }

        // Вроде всё хорошоу
        return true;


    #else // pre C++ 17

        std::vector<char> filedataVec;
        if (readFile(filename, filedataVec, pFileStat, ignoreSizeErrors))
        {
            filedata.assign(filedataVec.begin(), filedataVec.end());
            return true;
        }

        return false;

    #endif

}

//------------------------------
template<typename StringType, typename DataType> inline
bool writeFile( const StringType &filename       //!< Имя файла
              , const DataType   *pData          //!< Данные
              , size_t            dataSize
              , bool bOverwrite
              )
{
    HANDLE hFile = openFileForWrittingWin32(filename.c_str(), bOverwrite);
    if (hFile==INVALID_HANDLE_VALUE)
    {
        // DWORD err = GetLastError(); // not used
        return false;
    }

    if (!dataSize)
    {
        CloseHandle(hFile);
        return true;
    }

    const size_t itemSize = sizeof(DataType);
    const size_t numRawBytesToWrite = dataSize*itemSize;

    DWORD lpNumberOfBytesWritten = 0;

    BOOL bRes = WriteFile(hFile, (LPCVOID)pData, (DWORD)numRawBytesToWrite, &lpNumberOfBytesWritten, 0);
    CloseHandle(hFile);

    if (!bRes)
        return false;

    if (numRawBytesToWrite!=(size_t)lpNumberOfBytesWritten)
        return false;

    return true;
}

//------------------------------
template<typename StringType, typename DataType> inline
bool writeFile( const StringType            &filename    //!< Имя файла
              , const std::vector<DataType> &filedata    //!< Вектор для данных
              , bool                        bOverwrite
              )
{
    if (filedata.empty())
    {
        DataType d;
        return writeFile(filename, &d, 0, bOverwrite);
    }
    else
    {
        return writeFile(filename, &filedata[0], filedata.size(), bOverwrite);
    }
}

//------------------------------
template<typename StringType> inline
bool writeFile( const StringType            &filename    //!< Имя файла
              , const std::string           &filedata    //!< Вектор для данных
              , bool                        bOverwrite
              )
{
    return writeFile(filename, filedata.data(), filedata.size(), bOverwrite);
}

//------------------------------
//! Получение текущего рабочего каталога - специализация для std::string
template<> inline
std::string getCurrentDirectory<std::string>()
{
    typedef char          char_type;
    typedef std::string   string_type;
    // constexpr const std::size_t char_size = sizeof(char_type);

    DWORD size = ::GetCurrentDirectoryA(0, 0); // Узнаем необходимое количество символов
    if (size==0)
        return string_type();

    if (size<256) // терминирующий ноль включен в результат, но мы перестраховываемся, поэтому строго меньше
    {
        char_type buf[256];
        size = ::GetCurrentDirectoryA((DWORD)std::size(buf), &buf[0]);
        if (size==0)
            return string_type();

        return string_type(&buf[0], &buf[size]); // Если всё норм, то size не содержит завершающий ноль
    }

    std::vector<char_type> buf;
    buf.resize(size+1, 0); // Тут тоже перестраховка под Z-ноль. Сколько выделять - не важно, всё равно выделять в куче
    size = ::GetCurrentDirectoryA((DWORD)std::size(buf), &buf[0]);
    if (size==0)
        return string_type();

    return string_type(&buf[0], &buf[size]); // Если всё норм, то size не содержит завершающий ноль
}

//------------------------------
//! Получение текущего рабочего каталога - специализация для std::wstring
template<> inline
std::wstring getCurrentDirectory<std::wstring>()
{
    typedef wchar_t       char_type;
    typedef std::wstring  string_type;
    // constexpr const std::size_t char_size = sizeof(char_type);

    DWORD size = ::GetCurrentDirectoryW(0, 0); // Узнаем необходимое количество символов
    if (size==0)
        return string_type();

    if (size<256) // терминирующий ноль включен в результат, но мы перестраховываемся, поэтому строго меньше
    {
        char_type buf[256];
        size = ::GetCurrentDirectoryW((DWORD)std::size(buf), &buf[0]);
        if (size==0)
            return string_type();

        return string_type(&buf[0], &buf[size]); // Если всё норм, то size не содержит завершающий ноль
    }

    std::vector<char_type> buf;
    buf.resize(size+1, 0); // Тут тоже перестраховка под Z-ноль. Сколько выделять - не важно, всё равно выделять в куче
    size = ::GetCurrentDirectoryW((DWORD)std::size(buf), &buf[0]);
    if (size==0)
        return string_type();

    return string_type(&buf[0], &buf[size]); // Если всё норм, то size не содержит завершающий ноль
}

//----------------------------------------------------------------------------
//! Установка текущего рабочего каталога - специализация для std::string
template<> inline
bool setCurrentDirectory<std::string>(const std::string &newCurDir)
{
    return SetCurrentDirectoryA(newCurDir.c_str()) ? true : false;
}

//------------------------------
//! Установка текущего рабочего каталога - специализация для std::wstring
template<> inline
bool setCurrentDirectory<std::wstring>(const std::wstring &newCurDir)
{
    return SetCurrentDirectoryW(newCurDir.c_str()) ? true : false;
}

//----------------------------------------------------------------------------
template<typename EnumDirectoryHandler> inline
bool enumerateDirectoryImpl(std::wstring path, EnumDirectoryHandler handler)
{
    path = umba::filename::appendPath(path, std::wstring(L"*.*"));

    // stripLastPathSep(path);

    // https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
    // https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
    WIN32_FIND_DATAW fndData;

    //TODO: !!! При возникновении исключения hFind утекает. Надо бы переделать, и такие моменты надо бы везде это проверить
    HANDLE hFind = ::FindFirstFileW( umba::filename::prepareForNativeUsage(path).c_str(), &fndData );

    if (hFind==INVALID_HANDLE_VALUE)
    {
        return false;
    }

    std::wstring curDirAlias    = umba::filename::getNativeCurrentDirAlias<std::wstring>();
    std::wstring parentDirAlias = umba::filename::getNativeParentDirAlias <std::wstring>();
    auto isSpecialAlias = [&](const std::wstring &name) -> bool
    {
        return (name==curDirAlias || name==parentDirAlias);
    };

    FileStat fileStat;

    if (!isSpecialAlias(fndData.cFileName))
    {
        fileStat = fileStatFromFindData(fndData);
        if (!handler(umba::filesys::impl_helpers::encodeFromNative(fndData.cFileName), fileStat)) // (fndData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? true : false
        {
            return true;
        }
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextfilea
    while(::FindNextFileW(hFind, &fndData))
    {
        if (!isSpecialAlias(fndData.cFileName))
        {
            fileStat = fileStatFromFindData(fndData);
            if (!handler(umba::filesys::impl_helpers::encodeFromNative(fndData.cFileName), fileStat)) // (fndData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? true : false
            {
                return true;
            }
        }
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findclose
    ::FindClose(hFind);

    return true;
}

//----------------------------------------------------------------------------
template<typename EnumDirectoryHandler> inline
bool enumerateDirectoryImpl(std::string path, EnumDirectoryHandler handler)
{
    path = umba::filename::appendPath(path, std::string("*.*"));

    // stripLastPathSep(path);

    // https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
    // https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
    WIN32_FIND_DATAA fndData;

    //TODO: !!! При возникновении исключения hFind утекает. Надо бы везде это проверить
    //HANDLE hFind = ::FindFirstFileA( umba::filename::prepareForNativeUsage(path).c_str(), &fndData );
    HANDLE hFind = ::FindFirstFileA( path.c_str(), &fndData );

    if (hFind==INVALID_HANDLE_VALUE)
    {
        return false;
    }

    std::string curDirAlias    = umba::filename::getNativeCurrentDirAlias<std::string>();
    std::string parentDirAlias = umba::filename::getNativeParentDirAlias <std::string>();
    auto isSpecialAlias = [&](const std::string &name) -> bool
    {
        return (name==curDirAlias || name==parentDirAlias);
    };

    FileStat fileStat;

    if (!isSpecialAlias(fndData.cFileName))
    {
        fileStat = fileStatFromFindData(fndData);
        if (!handler(fndData.cFileName, fileStat)) // (fndData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? true : false
        {
            return true;
        }
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextfilea
    while(::FindNextFileA(hFind, &fndData))
    {
        if (!isSpecialAlias(fndData.cFileName))
        {
            fileStat = fileStatFromFindData(fndData);
            if (!handler(fndData.cFileName, fileStat)) // (fndData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ? true : false
            {
                return true;
            }
        }
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findclose
    ::FindClose(hFind);

    return true;
}

//----------------------------------------------------------------------------


#else // !WIN32


//----------------------------------------------------------------------------
//! Удаление файла, специализация для std::string
template<> inline
bool deleteFile<std::string>( const std::string &filename )
{
    // same as remove for files
    return unlink(filename.c_str()) == 0 ? true : false;
}

//----------------------------------------------------------------------------
//! Удаление каталога, специализация для std::string
template<> inline
bool deleteDirectory<std::string>( const std::string &filename )
{
    // same as remove for folders
    return rmdir(filename.c_str()) == 0 ? true : false;
}

//----------------------------------------------------------------------------
//! Проверка доступности файла на чтение
template<typename StringType> inline
bool isFileReadable( const StringType &filename )
{
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: isFileReadable not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Чтение файла в вектор
/*!
    \tparam StringType Тип имени файла - std::string / std::wstring
    \tparam DataType   Тип читаемых данных

    \return Возвращает true, если файл был прочитан
 */
template<typename StringType, typename DataType> inline
bool readFile( const StringType &filename, std::vector<DataType> &filedata
             , FileStat *pFileStat = 0          //!< [out] Статистика файла
             , bool ignoreSizeErrors = true     //!< Игнорировать разночтения из статистики файла и реально прочитанного размера
             )
{
    filedata.clear();

    FileStat fileStat = getFileStat( filename );
    if (fileStat.fileType!=FileTypeFile)
        return false;

    if (pFileStat)
        *pFileStat = fileStat;

    if (fileStat.fileSize==0)
        return true; // no data for reading at all

    if (!isFileReadable( filename ))
        return false;

    // Here starts "no exceptions" (exception safe) zone
    std::ifstream ifs;

    ifs.clear();
    ifs.open( filename.c_str() );
    if (!ifs)
    {
        filedata.clear(); // Ошибка. Если вектор не временный, то неплохо бы его обнулить, чтобы место в памяти не хавал.
        return false;
    }

    const size_t itemSize = sizeof(DataType);
    const size_t numItems = (size_t)(fileStat.fileSize / itemSize);
    filedata.resize( numItems ); // We can read files which are always can fit to memory
    const size_t numRawBytesToRead = filedata.size()*itemSize;

    ifs.read( (char*)&filedata[0], (std::size_t)numRawBytesToRead );

    if (ifs.fail())
    {
        filedata.clear(); // Ошибка. Если вектор не временный, то неплохо бы его обнулить, чтобы место в памяти не хавал.
        return false;
    }

    // Except in the constructors of std::strstreambuf, negative values of std::streamsize are never used.
    size_t readedBytes = (size_t)ifs.gcount();

    if (readedBytes!=(size_t)fileStat.fileSize)
    {
        if (ignoreSizeErrors)
        {
            fileStat.fileSize = (filesize_t)filedata.size(); // сохраняем консистенцию размера вектора и fileStat'а. Или не надо?
            return true;
        }

        // Чистим вектор, и возвращаем ошибку
        filedata.clear();
        return false;
    }

    // Вроде всё хорошоу
    return true;

}

#if 0
// Not currently implemented
//------------------------------
template<typename StringType, typename DataType> inline
bool writeFile( const StringType            &filename    //!< Имя файла
              , const std::vector<DataType> &filedata    //!< Вектор для данных
              , bool                        bOverwrite
              )

//------------------------------
template<typename StringType, typename DataType> inline
bool writeFile( const StringType &filename       //!< Имя файла
              , const DataType   *pData          //!< Данные
              , size_t            dataSize
              , bool bOverwrite
              )
#endif
//----------------------------------------------------------------------------
//! Проверка доступности файла на чтение - специализация для std::string
template<> inline
bool isFileReadable<std::string>( const std::string &filename )
{
    if (filename.empty()) return false;

    std::ifstream ifs;

    ifs.clear();
    ifs.open( filename.c_str() );
    if (!!ifs)
    {
        foundName = fileName;
        ifs.close();
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
//! Чтение файла в вектор - специализация
template<> inline
bool readFile<std::wstring, uint8_t>( const std::wstring &filename, std::vector<uint8_t> &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true /* read something */ )
{
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: readFile not specialized for this StringType");
}

//----------------------------------------------------------------------------
//! Чтение файла в вектор - специализация
template<> inline
bool readFile<std::wstring, char>( const std::wstring &filename, std::vector<char> &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true /* read something */ )
{
    #ifdef UMBA_DEBUGBREAK
        UMBA_DEBUGBREAK();
    #endif
    throw std::runtime_error("Not implemented: readFile not specialized for this StringType");
}

//------------------------------
//! Получение текущего рабочего каталога - специализация для std::wstring
template<> inline
std::string getCurrentDirectory<std::string>()
   {
    size_t size = 1024;
    char *buf = (char*)_alloca(size*sizeof(buf[0]));
    char* pRes = getcwd( buf, int(size));
    while(!pRes && errno==ERANGE)
       {
        size *= 2;
        buf = (char*)_alloca(size*sizeof(buf[0]));
        pRes = getcwd( buf, int(size));
       }
    if (!pRes) return ::std::string();
    return ::std::string(pRes);
   }

//----------------------------------------------------------------------------

#endif // WIN32

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template<typename StringType, typename EnumDirectoryHandler> inline
bool enumerateDirectory(const StringType &path, EnumDirectoryHandler handler)
{
    return enumerateDirectoryImpl(path, handler);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Получение имени текущего диска
template<typename StringType> inline
StringType getCurrentDrive()
{
    #if defined(WIN32) || defined(_WIN32)

    namespace ustrp = umba::string_plus;

    StringType curDir = umba::filename::makeCanonical(getCurrentDirectory<StringType>());
    std::vector< StringType > parts = ustrp::split(curDir, umba::filename::template getNativePathSep<typename StringType::value_type>(), true /* skipEmpty */ );
    if (!parts.empty())
        return ustrp::toupper_copy(parts[0]);
    return StringType();

    #else

    return StringType();

    #endif
}

//----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)

    //------------------------------

    //! Получение домашнего каталога текущего пользователя в винде
    template<typename StringType>
    StringType getUserFolderPathFromWinApi(); // { return throw std::runtime_error("getUserFolderPathFromWinApi: not specified"); StringType(); }

    //! Получение домашнего каталога текущего пользователя в винде. Специализация
    template<> inline
    std::string getUserFolderPathFromWinApi<std::string>()
    {
        char buf[4096];

        if (SHGetSpecialFolderPathA( 0, buf, CSIDL_PROFILE, false))
        //if (SHGetFolderPathA( 0, buf, CSIDL_PROFILE, false))
           return buf;

        return std::string();
    }

    //! Получение домашнего каталога текущего пользователя в винде. Специализация
    template<> inline
    std::wstring getUserFolderPathFromWinApi<std::wstring>()
    {
        wchar_t buf[4096];

        if (SHGetSpecialFolderPathW( 0, buf, CSIDL_PROFILE, false))
        //if (SHGetFolderPathA( 0, buf, CSIDL_PROFILE, false))
           return buf;

        return std::wstring();
    }


    //------------------------------

    //! Получение временного каталога (текущего пользователя или как придётся) в винде
    template<typename StringType>
    StringType getTempFolderPathFromWinApi(); // { return throw std::runtime_error("getUserFolderPathFromWinApi: not specified"); StringType(); }

    //! Получение временного каталога (текущего пользователя или как придётся) в винде. Специализация
    template<> inline
    std::string getTempFolderPathFromWinApi<std::string>()
    {
        char buf[4096];

        const std::size_t bufSize = sizeof(buf) / sizeof(buf[0]);

        DWORD res = GetTempPathA( bufSize, &buf[0]);
        // Если 0 - это ошибка, если > размера буфера - не влезло - сорян, если не хватило 4Кб под имя фолдера, что-то пошло явно не так
        if (!res || res>bufSize)
            return std::string(); // std::string("C:\\Temp");

        return std::string( &buf[0], res );
    }

    //! Получение временного каталога (текущего пользователя или как придётся) в винде. Специализация
    template<> inline
    std::wstring getTempFolderPathFromWinApi<std::wstring>()
    {
        wchar_t buf[4096];

        const std::size_t bufSize = sizeof(buf) / sizeof(buf[0]);

        DWORD res = GetTempPathW( bufSize, &buf[0]);
        // Если 0 - это ошибка, если > размера буфера - не влезло - сорян, если не хватило 4Кб под имя фолдера, что-то пошло явно не так
        if (!res || res>bufSize)
            return std::wstring(); // std::wstring(L"C:\\Temp");

        return std::wstring( &buf[0], res );
    }

#endif // defined(WIN32) || defined(_WIN32)

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Получение домашнего каталога текущего пользователя
template<typename StringType> inline
StringType getCurrentUserHomeDirectory()
{
    #if defined(WIN32) || defined(_WIN32)

        StringType res;
        if (getVar(umba::string_plus::make_string<StringType>("USERPROFILE"), res))
        {
            return res;
        }

        return getUserFolderPathFromWinApi<StringType>();

    #else

        //TODO: Переделать !!!
        // https://stackoverflow.com/questions/2910377/get-home-directory-in-linux

        StringType res;
        if (getVar(make_string<StringType>("HOME"), res))
        {
            return res;
        }

        return umba::string_plus::make_string<StringType>(getpwuid(getuid())->pw_dir);

    #endif

}

//----------------------------------------------------------------------------
//! Получение TEMP каталога
template<typename StringType> inline
StringType getTempFolderPath()
{
    #if defined(WIN32) || defined(_WIN32)

        StringType res = getTempFolderPathFromWinApi<StringType>();

        if (!res.empty())
            return res;

        if (getVar(string::make_string<StringType>("TEMP"), res))
        {
            return res;
        }

        if (getVar(string::make_string<StringType>("TMP"), res))
        {
            return res;
        }

        return string::make_string<StringType>("C:\\Temp");

    #else

        // Пытаемся найти юзерский TEMP
        StringType res;

        if (getVar(string::make_string<StringType>("TMPDIR"), res))
        {
            return res;
        }

        if (getVar(string::make_string<StringType>("TMP"), res))
        {
            return res;
        }

        return string::make_string<StringType>("/tmp");

    #endif
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------

// End of UMBA_FILESYSTEM
/*! @}*/

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace internal
} // namespace filesys
} // namespace umba

// umba::filesys::


#include "../filename.h"
