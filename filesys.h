/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Прослойка абстракций для работы с файловой системой, версия, работающая одинаково с любыми типами имён (char/wchar_t)

    Repository: https://github.com/al-martyn1/umba

    Все функции принимают как std::string, так и std::wstring, но возвращают всегда только std::string.

    В std::string юникод представлен в кодировке UTF-8.

    Надо подумать, и как-то объявить "umba/filesys.h" устаревшим заголовком, чтобы везде выдавались предупреждения при его использовании.
    Как вариант, спрятать "umba/filesys.h" в подкаталог internal. Переехать на эту тему везде
    Переименовать текущий umba::filesys в umba::filesys::internal. Переехать на эту тему везде
*/

#pragma once

//-----------------------------------------------------------------------------

#include "umba.h"
#include "utf.h"

#include "internal/filesys.h"
#include "internal/filesys_impl_helpers.h"


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
#define UMBA_FILESYS_USE_UTF8

//-----------------------------------------------------------------------------
#if defined(WIN32) || defined(_WIN32)

    #if !defined(UMBA_FILESYS_HAS_NATIVE_WIDE_API)
        #define UMBA_FILESYS_HAS_NATIVE_WIDE_API
    #endif

    //! Used as: const char *appExe    = "project" UMBA_FILESYS_EXE_EXT;
    #define UMBA_FILESYS_EXE_EXT  ".exe"

#else
    // #define UMBA_FILESYS_HAS_NATIVE_CHAR_API

    //! Used as: const char *appExe    = "project" UMBA_FILESYS_EXE_EXT;
    #define UMBA_FILESYS_EXE_EXT  ""

#endif

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// umba::filesys::
namespace umba{
namespace filesys{

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Aliasing for current implementation
namespace fsysapi = ::umba::filesys::internal;

#if defined(UMBA_FILESYS_HAS_NATIVE_WIDE_API)

    using native_fs_string_t = std::wstring;

#else // !UMBA_FILESYS_HAS_NATIVE_WIDE_API

    using native_fs_string_t = std::string;

#endif // UMBA_FILESYS_HAS_NATIVE_WIDE_API


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using impl_helpers::nameConvert;
using impl_helpers::encodeToNative;
using impl_helpers::encodeFromNative;

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Here goes API itself


//-----------------------------------------------------------------------------
// Use fsysapi::, Luke


//-----------------------------------------------------------------------------
using FileType = fsysapi::FileType; // пофиг, что проросло, надо переделывать

// enum FileType // Надо бы переделать в enum class, но уж больно много где проросло
// {
//     FileTypeInvalid = fsysapi::FileTypeInvalid, //!< Файл не найден, или ещё какая-то проблема
//     FileTypeUnknown = fsysapi::FileTypeUnknown, //!< Тип файла не известен
//     FileTypeDir     = fsysapi::FileTypeDir    , //!< Каталог, не файл
//     FileTypeFile    = fsysapi::FileTypeFile   , //!< Файл
//     FileTypeRam     = fsysapi::FileTypeRam      //!< Что-то просто лежит в памяти
// };




//-----------------------------------------------------------------------------
// Мы реализуем полную замену предыдущего API, так что делаем и те функции, которые от имён не зависят

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
template<typename DataType> inline
bool readFile(std::istream &fileIn, std::vector<DataType> &filedata)
{
    return fsysapi::readFile(fileIn, filedata);
}

//-----------------------------------------------------------------------------
inline bool readFile(std::istream &fileIn, std::string &filedata)
{
    return fsysapi::readFile(fileIn, filedata);
}

//-----------------------------------------------------------------------------
template<typename DataType> inline
bool writeFile(std::ostream &fileOut, const DataType *pData, size_t dataSize)
{
    return fsysapi::writeFile(fileOut, pData, dataSize);
}

//-----------------------------------------------------------------------------
template<typename DataType> inline
bool writeFile(std::ostream &fileOut, const std::vector<DataType> &data)
{
    return fsysapi::writeFile(fileOut, data);
}

//-----------------------------------------------------------------------------
inline bool writeFile(std::ostream &fileOut, const std::string &data)
{
    return fsysapi::writeFile(fileOut, data);
}

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using filetime_t = fsysapi::filetime_t;
using filesize_t = fsysapi::filesize_t;

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Use impl_helpers::encodeToNative and impl_helpers::encodeFromNative, Luke

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//! Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
inline std::string formatFiletime(filetime_t t, const std::wstring &fmt)
{
    return impl_helpers::encodeFromNative(fsysapi::formatFiletime(t, impl_helpers::encodeToNative(fmt)));
}

//-----------------------------------------------------------------------------
//! Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
inline std::string formatFiletime(filetime_t t, const std::string &fmt)
{
    return impl_helpers::encodeFromNative(fsysapi::formatFiletime(t, impl_helpers::encodeToNative(fmt)));
}

//-----------------------------------------------------------------------------
//! Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
inline std::string formatFiletime(filetime_t t, const wchar_t *fmt)
{
    return impl_helpers::encodeFromNative(fsysapi::formatFiletime(t, impl_helpers::encodeToNative(fmt)));
}

//-----------------------------------------------------------------------------
//! Описание форматной строки тут - https://man7.org/linux/man-pages/man3/strftime.3.html
inline std::string formatFiletime(filetime_t t, const char *fmt)
{
    return impl_helpers::encodeFromNative(fsysapi::formatFiletime(t, impl_helpers::encodeToNative(fmt)));
}

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
using FileStat = fsysapi::FileStat;

//-----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Возвращает FileStat по пути (файл или каталог, не важно)
inline bool getPathStat(const std::wstring &path, FileStat &fileStat)
{
    return fsysapi::getPathStat(impl_helpers::encodeToNative(path), fileStat);
}

//------------------------------
//! Возвращает FileStat по пути (файл или каталог, не важно)
inline bool getPathStat(const std::string &path, FileStat &fileStat)
{
    return fsysapi::getPathStat(impl_helpers::encodeToNative(path), fileStat);
}

//------------------------------
//! Возвращает FileStat по пути (файл или каталог, не важно)
inline bool getPathStat(const wchar_t *path, FileStat &fileStat)
{
    return fsysapi::getPathStat(impl_helpers::encodeToNative(path), fileStat);
}

//------------------------------
//! Возвращает FileStat по пути (файл или каталог, не важно)
inline bool getPathStat(const char *path, FileStat &fileStat)
{
    return fsysapi::getPathStat(impl_helpers::encodeToNative(path), fileStat);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isPathExist(const std::wstring &path)
{
    return fsysapi::isPathExist(impl_helpers::encodeToNative(path));
}

//------------------------------

inline bool isPathExist(const std::string &path)
{
    return fsysapi::isPathExist(impl_helpers::encodeToNative(path));
}

//------------------------------

inline bool isPathExist(const wchar_t *path)
{
    return fsysapi::isPathExist(impl_helpers::encodeToNative(path));
}

//------------------------------

inline bool isPathExist(const char *path)
{
    return fsysapi::isPathExist(impl_helpers::encodeToNative(path));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isPathDirectory(const std::wstring &path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathDirectory(const std::string &path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathDirectory(const wchar_t *path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathDirectory(const char *path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
inline bool isPathDir(const std::wstring &path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathDir(const std::string &path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathDir(const wchar_t *path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathDir(const char *path)
{
    return fsysapi::isPathDirectory(impl_helpers::encodeToNative(path));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isPathFile(const std::wstring &path)
{
    return fsysapi::isPathFile(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathFile(const std::string &path)
{
    return fsysapi::isPathFile(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathFile(const wchar_t *path)
{
    return fsysapi::isPathFile(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isPathFile(const char *path)
{
    return fsysapi::isPathFile(impl_helpers::encodeToNative(path));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isFileExist(const std::wstring &path)
{
    return fsysapi::isFileExist(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isFileExist(const std::string &path)
{
    return fsysapi::isFileExist(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isFileExist(const wchar_t *path)
{
    return fsysapi::isFileExist(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isFileExist(const char *path)
{
    return fsysapi::isFileExist(impl_helpers::encodeToNative(path));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isDirExist(const std::wstring &path)
{
    return fsysapi::isDirExist(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isDirExist(const std::string &path)
{
    return fsysapi::isDirExist(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isDirExist(const wchar_t *path)
{
    return fsysapi::isDirExist(impl_helpers::encodeToNative(path));
}

//------------------------------
inline bool isDirExist(const char *path)
{
    return fsysapi::isDirExist(impl_helpers::encodeToNative(path));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isFileReadonly( const std::wstring &fname )
{
    return fsysapi::isFileReadonly(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileReadonly( const std::string &fname )
{
    return fsysapi::isFileReadonly(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileReadonly( const wchar_t *fname )
{
    return fsysapi::isFileReadonly(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileReadonly( const char *fname )
{
    return fsysapi::isFileReadonly(impl_helpers::encodeToNative(fname));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isFileReadable( const std::wstring &fname )
{
    return fsysapi::isFileReadable(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileReadable( const std::string &fname )
{
    return fsysapi::isFileReadable(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileReadable( const wchar_t *fname )
{
    return fsysapi::isFileReadable(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileReadable( const char *fname )
{
    return fsysapi::isFileReadable(impl_helpers::encodeToNative(fname));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool isFileExistingExclusiveReadableWrittable( const std::wstring &fname )
{
    return fsysapi::isFileExistingExclusiveReadableWrittable(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileExistingExclusiveReadableWrittable( const std::string &fname )
{
    return fsysapi::isFileExistingExclusiveReadableWrittable(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileExistingExclusiveReadableWrittable( const wchar_t *fname )
{
    return fsysapi::isFileExistingExclusiveReadableWrittable(impl_helpers::encodeToNative(fname));
}

//------------------------------
inline bool isFileExistingExclusiveReadableWrittable( const char *fname )
{
    return fsysapi::isFileExistingExclusiveReadableWrittable(impl_helpers::encodeToNative(fname));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline filetime_t getFileTimeNow()
{
    return fsysapi::getFileTimeNow();
}

//----------------------------------------------------------------------------
inline bool isLastErrorAlreadyExists()
{
    return fsysapi::isLastErrorAlreadyExists();
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline std::string getTempFolderPath()
{
    return impl_helpers::encodeFromNative(fsysapi::getTempFolderPath<native_fs_string_t>());
}

//----------------------------------------------------------------------------
inline std::string getCurrentUserHomeDirectory()
{
    return impl_helpers::encodeFromNative(fsysapi::getCurrentUserHomeDirectory<native_fs_string_t>());
}

//----------------------------------------------------------------------------
inline std::string getCurrentDrive()
{
    return impl_helpers::encodeFromNative(fsysapi::getCurrentDrive<native_fs_string_t>());
}

//----------------------------------------------------------------------------
//! Получение текущего рабочего каталога
inline std::string getCurrentDirectory()
{
    return impl_helpers::encodeFromNative(fsysapi::getCurrentDirectory<native_fs_string_t>());
}

//----------------------------------------------------------------------------
//! Получение текущего рабочего каталога
inline std::string getCurrentDir() { return getCurrentDirectory(); }

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDirectory(const std::wstring &newCurDir)
{
    return fsysapi::setCurrentDirectory(impl_helpers::encodeToNative(newCurDir));
}

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDirectory(const std::string &newCurDir)
{
    return fsysapi::setCurrentDirectory(impl_helpers::encodeToNative(newCurDir));
}

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDirectory(const wchar_t *newCurDir)
{
    return fsysapi::setCurrentDirectory(impl_helpers::encodeToNative(newCurDir));
}

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDirectory(const char *newCurDir)
{
    return fsysapi::setCurrentDirectory(impl_helpers::encodeToNative(newCurDir));
}

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDir(const std::wstring &newCurDir) { return setCurrentDirectory(newCurDir); }

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDir(const std::string &newCurDir)  { return setCurrentDirectory(newCurDir); }

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDir(const wchar_t *newCurDir)      { return setCurrentDirectory(newCurDir); }

//------------------------------
//! Установка текущего рабочего каталога
inline bool setCurrentDir(const char *newCurDir)         { return setCurrentDirectory(newCurDir); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Перемещение (переименование) файла или каталога
inline bool moveFileOrDirectory(const std::string &oldName, const std::string &newName)
{
    return fsysapi::moveFileOrDirectory(impl_helpers::encodeToNative(oldName), impl_helpers::encodeToNative(newName));
}

//------------------------------
//! Перемещение (переименование) файла или каталога
inline bool moveFileOrDirectory(const std::wstring &oldName, const std::wstring &newName)
{
    return fsysapi::moveFileOrDirectory(impl_helpers::encodeToNative(oldName), impl_helpers::encodeToNative(newName));
}

//------------------------------
//! Перемещение (переименование) файла или каталога
inline bool moveFileOrDir(const std::string &oldName, const std::string &newName)   { return moveFileOrDirectory(oldName, newName); }

//------------------------------
//! Перемещение (переименование) файла или каталога
inline bool moveFileOrDir(const std::wstring &oldName, const std::wstring &newName) { return moveFileOrDirectory(oldName, newName); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Перемещение (переименование) файла или каталога
inline bool copyFile(const std::string &oldName, const std::string &newName, bool bOverwrite)
{
    return fsysapi::copyFile(impl_helpers::encodeToNative(oldName), impl_helpers::encodeToNative(newName), bOverwrite);
}

//------------------------------
//! Перемещение (переименование) файла или каталога
inline bool copyFile(const std::wstring &oldName, const std::wstring &newName, bool bOverwrite)
{
    return fsysapi::copyFile(impl_helpers::encodeToNative(oldName), impl_helpers::encodeToNative(newName), bOverwrite);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Удаление файла
inline bool deleteFile( const std::wstring &filename )
{
    return fsysapi::deleteFile(impl_helpers::encodeToNative(filename));
}

//------------------------------
//! Удаление файла
inline bool deleteFile( const std::string &filename )
{
    return fsysapi::deleteFile(impl_helpers::encodeToNative(filename));
}

//------------------------------
//! Удаление файла
inline bool deleteFile( const wchar_t *filename )
{
    return fsysapi::deleteFile(impl_helpers::encodeToNative(filename));
}

//------------------------------
//! Удаление файла
inline bool deleteFile( const char *filename )
{
    return fsysapi::deleteFile(impl_helpers::encodeToNative(filename));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Удаление каталога
inline bool deleteDirectory( const std::wstring &dirname )
{
    return fsysapi::deleteDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Удаление каталога
inline bool deleteDirectory( const std::string &dirname )
{
    return fsysapi::deleteDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Удаление каталога
inline bool deleteDirectory( const wchar_t *dirname )
{
    return fsysapi::deleteDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Удаление каталога
inline bool deleteDirectory( const char *dirname )
{
    return fsysapi::deleteDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Удаление каталога
inline bool deleteDir( const std::wstring &dirname ) { return deleteDirectory(dirname); }

//------------------------------
//! Удаление каталога
inline bool deleteDir( const std::string &dirname )  { return deleteDirectory(dirname); }

//------------------------------
//! Удаление каталога
inline bool deleteDir( const wchar_t *dirname )      { return deleteDirectory(dirname); }

//------------------------------
//! Удаление каталога
inline bool deleteDir( const char *dirname )         { return deleteDirectory(dirname); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Создание каталога
inline bool createDirectory( const std::wstring &dirname )
{
    return fsysapi::createDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Создание каталога
inline bool createDirectory( const std::string &dirname )
{
    return fsysapi::createDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Создание каталога
inline bool createDirectory( const wchar_t *dirname )
{
    return fsysapi::createDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Создание каталога
inline bool createDirectory( const char *dirname )
{
    return fsysapi::createDirectory(impl_helpers::encodeToNative(dirname));
}

//------------------------------
//! Создание каталога
inline bool createDir( const std::wstring &dirname )  { return createDirectory(dirname); }

//------------------------------
//! Создание каталога
inline bool createDir( const std::string &dirname )   { return createDirectory(dirname); }

//------------------------------
//! Создание каталога
inline bool createDir( const wchar_t *dirname )       { return createDirectory(dirname); }

//------------------------------
//! Создание каталога
inline bool createDir( const char *dirname )          { return createDirectory(dirname); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirectoryEx( const std::wstring &dirname, bool forceCreatePath )
{
    return fsysapi::createDirectoryEx(impl_helpers::encodeToNative(dirname), forceCreatePath);
}

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirectoryEx( const std::string &dirname, bool forceCreatePath )
{
    return fsysapi::createDirectoryEx(impl_helpers::encodeToNative(dirname), forceCreatePath);
}

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirectoryEx( const wchar_t *dirname, bool forceCreatePath )
{
    return fsysapi::createDirectoryEx(impl_helpers::encodeToNative(dirname), forceCreatePath);
}

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirectoryEx( const char *dirname, bool forceCreatePath )
{
    return fsysapi::createDirectoryEx(impl_helpers::encodeToNative(dirname), forceCreatePath);
}

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirEx( const std::wstring &dirname, bool forceCreatePath ) { return createDirectoryEx(dirname, forceCreatePath); }

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirEx( const std::string &dirname, bool forceCreatePath )  { return createDirectoryEx(dirname, forceCreatePath); }

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirEx( const wchar_t *dirname, bool forceCreatePath )      { return createDirectoryEx(dirname, forceCreatePath); }

//------------------------------
//! Создание каталога с возможностью создания полного пути
inline bool createDirEx( const char *dirname, bool forceCreatePath )         { return createDirectoryEx(dirname, forceCreatePath); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename DataType> inline
bool readFile( const std::wstring &filename, std::vector<DataType> &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//------------------------------
template<typename DataType> inline
bool readFile( const std::string &filename, std::vector<DataType> &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//------------------------------
template<typename DataType> inline
bool readFile( const wchar_t *filename, std::vector<DataType> &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//------------------------------
template<typename DataType> inline
bool readFile( const char *filename   , std::vector<DataType> &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool readFile( const std::wstring &filename, std::string &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//------------------------------
inline bool readFile( const std::string &filename, std::string &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//------------------------------
inline bool readFile( const wchar_t *filename, std::string &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//------------------------------
inline bool readFile( const char *filename   , std::string &filedata, FileStat *pFileStat = 0, bool ignoreSizeErrors = true)
{
    return fsysapi::readFile(impl_helpers::encodeToNative(filename), filedata, pFileStat, ignoreSizeErrors);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename DataType> inline
bool writeFile( const std::wstring &filename, const DataType *pData, size_t dataSize, bool bOverwrite = false)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), pData, dataSize, bOverwrite);
}

//------------------------------
template<typename DataType> inline
bool writeFile( const std::string &filename, const DataType *pData, size_t dataSize, bool bOverwrite = false)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), pData, dataSize, bOverwrite);
}

//------------------------------
template<typename DataType> inline
bool writeFile( const wchar_t *filename, const DataType *pData, size_t dataSize, bool bOverwrite = false)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), pData, dataSize, bOverwrite);
}

//------------------------------
template<typename DataType> inline
bool writeFile( const char *filename, const DataType *pData, size_t dataSize, bool bOverwrite = false)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), pData, dataSize, bOverwrite);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename DataType> inline
bool writeFile( const std::wstring &filename, const std::vector<DataType> &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//------------------------------
template<typename DataType> inline
bool writeFile( const std::string &filename, const std::vector<DataType> &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//------------------------------
template<typename DataType> inline
bool writeFile( const wchar_t *filename, const std::vector<DataType> &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//------------------------------
template<typename DataType> inline
bool writeFile( const char *filename, const std::vector<DataType> &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline bool writeFile( const std::wstring &filename, const std::string &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//------------------------------
inline bool writeFile( const std::string &filename, const std::string &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//------------------------------
inline bool writeFile( const wchar_t *filename, const std::string &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//------------------------------
inline bool writeFile( const char *filename, const std::string &filedata, bool bOverwrite)
{
    return fsysapi::writeFile(impl_helpers::encodeToNative(filename), filedata, bOverwrite);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename EnumDirectoryHandler> inline
bool enumerateDirectory(const std::wstring &path, EnumDirectoryHandler handler)
{
    return fsysapi::enumerateDirectory(impl_helpers::encodeToNative(path), handler);
}

//------------------------------
template<typename EnumDirectoryHandler> inline
bool enumerateDirectory(const std::string &path, EnumDirectoryHandler handler)
{
    return fsysapi::enumerateDirectory(impl_helpers::encodeToNative(path), handler);
}

//------------------------------
template<typename EnumDirectoryHandler> inline
bool enumerateDirectory(const wchar_t *path, EnumDirectoryHandler handler)
{
    return fsysapi::enumerateDirectory(impl_helpers::encodeToNative(path), handler);
}

//------------------------------
template<typename EnumDirectoryHandler> inline
bool enumerateDirectory(const char *path, EnumDirectoryHandler handler)
{
    return fsysapi::enumerateDirectory(impl_helpers::encodeToNative(path), handler);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

// Use fsysapi::, Luke
// Use impl_helpers::encodeToNative and impl_helpers::encodeFromNative, Luke
// native_fs_string_t

//-----------------------------------------------------------------------------

} // namespace filesys
} // namespace umba




