/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Утилиты для работы с именами файлов

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

//-----------------------------------------------------------------------------

#include "critical_section.h"
#include "filesys.h"
#include "stl.h"
#include "string_plus.h"

//
#include <algorithm>
#include <iterator>

//#include "splits.h"
//#include "isa.h"

#include "zz_inc_windows.h"


// umba::filename::
namespace umba
{
namespace filename
{

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


template<typename StringType> inline
StringType getFileName( const StringType &path );
std::string  getFileName( const char    *p );
std::wstring getFileName( const wchar_t *p );



//-----------------------------------------------------------------------------
template<typename StringType> inline  StringType getNativeCurrentDirAlias( ) { return umba::string_plus::make_string<StringType>(".") ; } //!< Возвращает строку с алиасом текущего каталога, как это принято в целевой системе. Обычно это "."
template<typename StringType> inline  StringType getNativeParentDirAlias( )  { return umba::string_plus::make_string<StringType>(".."); } //!< Возвращает строку с алиасом родительского каталога, как это принято в целевой системе. Обычно это ".."
template<typename StringType> inline  StringType getNativeHomeDirAlias( )    { return umba::string_plus::make_string<StringType>("~") ; } //!< Возвращает строку с алиасом домашнего каталога текущего пользователя

template<typename CharType>   inline CharType getNativeExtSep( )    { return (CharType)'.'; } //!< Возвращает символ - разделитель расширения файла
//template<typename StringType> inline StringType getNativeExtSep( )  { return StringType(1, getNativeExtSep<typename StringType::value_type>() ); }

//! Возвращает символ - разделитель имён файлов в пути
template<typename CharType>
inline CharType getNativePathSep()
{
    #if defined(WIN32) || defined(_WIN32)
    return (CharType)'\\';
    #else
    return (CharType)'/';
    #endif
}

//! Возвращает символ - разделитель путей в списке
template<typename CharType>
inline CharType getNativePathListSep( )
{
    #if defined(WIN32) || defined(_WIN32)
    return (CharType)';';
    #else
    return (CharType)':';
    #endif
}

//template<typename StringType> inline StringType getNativePathSep( )      { return StringType(1, getNativePathSep<typename StringType::value_type>() ); }
//template<typename StringType> inline StringType getNativePathListSep( )  { return StringType(1, getNativePathListSep<typename StringType::value_type>() ); }


//-----------------------------------------------------------------------------
// Пути в Windows
// \\server\path\to\share\ - сетевой путь
// C:\windows\system32 - локальный путь
// \\?\C:\windows\system32
// \\?\UNC\server\share
// префикс "\\?\" нельзя использовать с относительным путем
// 
// Именование файлов, путей и пространства имён - https://learn.microsoft.com/ru-ru/windows/win32/fileio/naming-a-file
// Ограничение максимальной длины пути - https://learn.microsoft.com/ru-ru/windows/win32/fileio/maximum-file-path-limitation?tabs=registry

//! Возвращает UNC префикс (на самом деле правильно называть этот префикс Extended-Length Path или Win32 File Namespace)
template<typename StringType> inline
StringType getNativeUncPrefix()
{
    #if defined(WIN32) || defined(_WIN32)
    return umba::string_plus::make_string<StringType>("\\\\?\\");
    #else
    return StringType();
    #endif
}

//! Возвращает сетевой UNC префикс
template<typename StringType> inline
StringType getNativeNetworkUncPrefix()
{
    #if defined(WIN32) || defined(_WIN32)
    return umba::string_plus::make_string<StringType>("\\\\?\\UNC\\");
    #else
    return StringType();
    #endif
}

//-----------------------------------------------------------------------------
//! Возвращает сетевой префикс
template<typename StringType> inline
StringType getNativeNetworkPathPrefix()
{
    return umba::string_plus::make_string<StringType>( getNativePathSep<typename StringType::value_type>(), 2 );
}

//! Возвращает true, если путь начинается с сетевого префикса
template<typename StringType> inline
bool hasNativeNetworkPathPrefix( StringType &p )
{
    namespace ustrp = umba::string_plus;
    if (ustrp::starts_with(p, getNativeNetworkPathPrefix<StringType>()))
        return true;
    return false;
}

//! Возвращает true, если путь начинается с UNC префикса, обрезая его
template<typename StringType> inline
StringType stripNativeUncPrefix( StringType &p )
{
    namespace ustrp = umba::string_plus;
    ustrp::starts_with_and_strip(p, getNativeNetworkUncPrefix<StringType>());
    ustrp::starts_with_and_strip(p, getNativeUncPrefix<StringType>());
    return p;
}

struct NativePrefixFlagsInfo
{
    bool networkUncPrefix = false;
    bool uncPrefix        = false;
    bool netPrefix        = false;

    bool hasAnyPrefix() const
    {
        return networkUncPrefix || uncPrefix || netPrefix;
    }

}; // struct NativePrefixFlagsInfo

template<typename StringType> inline
NativePrefixFlagsInfo stripNativePrefixes(StringType &fileName, typename StringType::value_type pathSep);

// Добавляет нативные префиксы
template<typename StringType> inline
StringType addNativePrefixes(const StringType &fileName, const NativePrefixFlagsInfo &npfi);


//-----------------------------------------------------------------------------
template<typename CharType> inline bool isExtSep  ( CharType ch ) { return ch==(CharType)'.'; }                       //!< Возвращает true, если символ - разделитель расширения
template<typename CharType> inline bool isPathSep ( CharType ch ) { return ch==(CharType)'/' || ch==(CharType)'\\'; } //!< Возвращает true, если символ - разделитель пути
template<typename CharType> inline bool isDriveSep( CharType ch )                                                     //!< Возвращает true, если символ - разделитель буквы диска
{
#if defined(WIN32) || defined(_WIN32)
    if (ch==(CharType)':')
        return true;
#endif

    return false;
}

template<typename CharType> inline bool isPathOrDriveSep( CharType ch ) { return isPathSep(ch) || isDriveSep(ch); }   //!< Возвращает true, если символ - разделитель пути или буквы диска 

template<typename CharType> inline bool isAdsSep( CharType ch )                                                     //!< Возвращает true, если символ - разделитель Alternate Data Streams, ADS
{
#if defined(WIN32) || defined(_WIN32)
    if (ch==(CharType)':')
        return true;
#endif

    return false;
}


//-----------------------------------------------------------------------------
template<typename StringType> inline bool hasLastPathSep( StringType &p )   { return (p.empty() || !isPathSep(p[p.size()-1])) ? false : true; }                                              //!< Возвращает true, если последний символ - разделитель пути
template<typename StringType> inline bool stripLastPathSep( StringType &p ) { if (hasLastPathSep(p)) { p.erase( p.size()-1, 1 ); return true; } return false; }                              //!< Возвращает true, если последний символ - разделитель пути, обрезая его
template<typename StringType> inline StringType stripLastPathSepCopy( const StringType &p ) { if (!hasLastPathSep(p)) return p; StringType res = p; stripLastPathSep(res); return res; }     //!< Возвращает копию аргумента, обрезая разделитель пути в конце, если он есть
template<typename StringType> inline bool hasFirstPathSep( StringType &p )  { return (p.empty() || !isPathSep(p[0])) ? false : true; }                                                       //!< Возвращает true, если первый символ - разделитель пути
template<typename StringType> inline bool stripFirstPathSep(StringType &p)  { if (hasFirstPathSep(p)) { p.erase( 0, 1 ); return true; } return false; }                                                        //!< Возвращает true, если первый символ - разделитель пути, обрезая его
template<typename StringType> inline StringType stripFirstPathSepCopy( const StringType &p ) { if (!hasFirstPathSep(p)) return p; StringType res = p; stripFirstPathSep(res); return res; }  //!< Возвращает копию аргумента, обрезая разделитель пути в начале, если он есть

template<typename StringType> inline bool hasLastExtSep( StringType &p )    { return (p.empty() || !isExtSep(p[p.size()-1])) ? false : true; }                                               //!< Возвращает true, если последний символ - разделитель расширения
template<typename StringType> inline bool stripLastExtSep( StringType &p )  { if (hasLastExtSep(p)) { p.erase( p.size()-1, 1 ); return true; } return false; }                               //!< Возвращает true, если последний символ - разделитель расширения, обрезая его
template<typename StringType> inline StringType stripLastExtSepCopy( const StringType &p ) { if (!hasLastExtSep(p)) return p; StringType res = p; stripLastExtSep(res); return res; }        //!< Возвращает копию аргумента, обрезая разделитель расширения в конце, если он есть
template<typename StringType> inline bool hasFirstExtSep( StringType &p )   { return (p.empty() || !isExtSep(p[0])) ? false : true; }                                                        //!< Возвращает true, если первый символ - разделитель расширения
template<typename StringType> inline bool stripFirstExtSep(StringType &p)   { if (hasFirstExtSep(p)) { p.erase( 0, 1 ); return true; } return false; }                                       //!< Возвращает true, если первый символ - разделитель расширения, обрезая его
template<typename StringType> inline StringType stripFirstExtSepCopy( const StringType &p ) { if (!hasFirstExtSep(p)) return p; StringType res = p; stripFirstExtSep(res); return res; }     //!< Возвращает копию аргумента, обрезая разделитель расширения в начале, если он есть



//-----------------------------------------------------------------------------
//! Добавляет разделитель пути, если его нет в конце
template<typename StringType> inline
void appendPathSepInline( StringType &p, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
{
    if (!hasLastPathSep<StringType>(p)) p.append(1,pathSep);
}

//-----------------------------------------------------------------------------
//! Добавляет разделитель пути в возвращаемую копию аргумента, если его нет в конце
template<typename StringType> inline
StringType appendPathSepCopy( const StringType &p, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
{
    StringType pCopy = p; appendPathSepInline<StringType>(pCopy, pathSep); return pCopy;
}

//-----------------------------------------------------------------------------
//! Нормализует разделители путей
template<typename StringType> inline
StringType normalizePathSeparators( StringType fileName, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
{

    std::replace_if( fileName.begin(), fileName.end(), isPathSep<typename StringType::value_type>, pathSep );

    // for(auto &ch : fileName)
    // {
    //     if (isPathSep(ch))
    //         ch = pathSep;
    // }

    return fileName;
}

//-----------------------------------------------------------------------------
//! Возвращает true, если в пути содержаться разделители пути
template<typename StringType> inline
StringType hasPathSeparators( const StringType &fileName )
{
    for(auto ch : fileName)
    {
        if (isPathSep(ch))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//!
template<typename StringType> inline
std::vector< StringType > splitPath(StringType path)
{
    stripLastPathSep(path);

    auto pathSep = getNativePathSep<typename StringType::value_type>();

    path = normalizePathSeparators(path, pathSep);
    NativePrefixFlagsInfo npfi = stripNativePrefixes(path, pathSep);

    std::vector< StringType > parts;
    std::string curPart;
    for(auto ch: path)
    {
        if (ch==pathSep)
        {
            if (!curPart.empty())
            {
                parts.emplace_back(curPart);
                curPart.clear();
            }
        }
        else
        {
            curPart.append(1,ch);
        }
    }

    if (!curPart.empty())
    {
        parts.emplace_back(curPart);
    }

    if (!parts.empty())
    {
        parts[0] = addNativePrefixes(parts[0], npfi);
    }

    return parts;

}

//-----------------------------------------------------------------------------
//!
template<typename StringType> inline
StringType mergePath( const std::vector< StringType > &pathParts, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
{
    return umba::string_plus::merge(pathParts, pathSep);
}

//-----------------------------------------------------------------------------
//!
template<typename StringType> inline
StringType mergePath( const std::vector< StringType > &pathParts, std::size_t idxBegin, std::size_t idxEnd, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
{
    using pp_type = std::vector< StringType >;
    using pp_item_cptr = typename pp_type::const_pointer;

    if (idxEnd<=idxBegin)
        return StringType();

    auto stringifier = [](const auto &v) { return v; };
    return umba::string_plus::merge< StringType, pp_item_cptr, decltype(stringifier) >
                                   ( &pathParts[idxBegin], &pathParts[idxEnd], pathSep, stringifier )
                                   ;
}

//-----------------------------------------------------------------------------
//! Разделяет список путей
template<typename StringType> inline
std::vector< StringType > splitPathList( const StringType &pl, typename StringType::value_type pathListSep = getNativePathListSep<typename StringType::value_type>() )
{
    // Всегда используем ';' - для того, чтобы наши утилиты работали одинаково с данными, например, конфигов, одинаково - потому что конфиги пишутся под виндовую версию софта, 
    // и менять там что-то для релиза под другую ось в части списков путей - отдельные символы - нафиг не нужно
    // Но под *nix принят разделитель ':'
    // И *nix будут добавлять свои настройки с таким разделителем
    // Хотя ';' может быть под *nix валидным символом имени файла, обычно это используется для каких-то сугубо технических файлов
    // Так-то и в винде ';' - тоже нормальный символ для имени файла/каталога

    StringType sepList = StringType(1, (typename StringType::value_type)';');
    if (pathListSep!=0 && sepList[0]!=pathListSep)
    {
        sepList.append(1, pathListSep);
    }

    std::vector< StringType > resVec;

    typename StringType::size_type startPos = 0;
    typename StringType::size_type pos = pl.find_first_of( sepList, startPos );
    while(pos!=pl.npos)
    {
        if (pos!=startPos)
        {
            auto p = umba::string_plus::trim_copy(StringType(pl, startPos, pos-startPos));
            if (!p.empty())
                resVec.emplace_back(p);
        }

        startPos = pos+1;
        pos = pl.find_first_of( sepList, startPos );
    }

    if (startPos!=pl.npos && startPos!=pl.size())
    {
        auto p = umba::string_plus::trim_copy(StringType(pl, startPos, pl.size()-startPos));
        if (!p.empty())
            resVec.emplace_back(p);
    }

    return resVec;

    //return umba::string_plus::split( pl, pathListSep, true /* skipEmpty */ );
}

//-----------------------------------------------------------------------------
//!
template<typename StringType> inline
StringType mergePathList( const std::vector< StringType > &plParts, typename StringType::value_type pathListSep = getNativePathListSep<typename StringType::value_type>())
{
    using pp_type      = std::vector< StringType >;
    using pp_item_cptr = typename pp_type::const_iterator;

    auto stringifier = [](const StringType&v) { return v; };
    return umba::string_plus::merge< StringType, pp_item_cptr, decltype(stringifier) >
                                   ( plParts.begin(), plParts.end(), pathListSep, stringifier )
                                   ;
}


/*
template<typename StringType> inline
std::vector< StringType > getKeepCasePaths()
{
    static bool envReaded = false;
    static std::vector< StringType >  pathCache;

    if (!envReaded)
    {
        UMBA_CRITICAL_SECTION_EX( umba::CriticalSection, globalCriticalSection);
        if (!envReaded)
        {
        }
    }
}
*/

//-----------------------------------------------------------------------------
// Выделяем код обрезания различных спец префиксов в отдельные функции

#include "umba/warnings/push_disable_spectre_mitigation.h"
//! Отрезаем спец префиксы, ничего не трогая в имени, даже разделители путей
template<typename StringType> inline
NativePrefixFlagsInfo stripNativePrefixes(StringType &fileName, typename StringType::value_type pathSep)
{
    NativePrefixFlagsInfo npfi;

    // Почему-то ранее спец префиксы не переводились к текущему платформенному разделителю путей

    StringType nativeNetworkUncPrefix   = normalizePathSeparators(getNativeNetworkUncPrefix<StringType>()  , pathSep); // "\\?\UNC\"
    StringType nativeUncPrefix          = normalizePathSeparators(getNativeUncPrefix<StringType>()         , pathSep); // "\\?\"
    StringType nativeNetworkPathPrefix  = normalizePathSeparators(getNativeNetworkPathPrefix<StringType>() , pathSep); // "\\"

    //std::size_t numCharsStripped = 0;

    StringType fileNameTmp = normalizePathSeparators(fileName, pathSep);

    namespace ustrp = umba::string_plus;

    #if defined(WIN32) || defined(_WIN32)
    if (ustrp::starts_with_and_strip(fileNameTmp, nativeNetworkUncPrefix))
    {
        npfi.networkUncPrefix = true;
    }
    else if (ustrp::starts_with_and_strip(fileNameTmp, nativeUncPrefix))
    {
        npfi.uncPrefix = true;
    }
    #endif

    //bool hasNetPrefix = false;
    if (ustrp::starts_with_and_strip(fileNameTmp, nativeNetworkPathPrefix))
    {
        npfi.netPrefix = true;
    }

    if (fileNameTmp.size()<fileName.size())
    {
        std::size_t numCharsToStrip = fileName.size() - fileNameTmp.size();
        fileName.erase(0, numCharsToStrip);
    }

    return npfi;
}
#include "umba/warnings/pop.h"

// Добавляет нативные префиксы
template<typename StringType> inline
StringType addNativePrefixes(const StringType &fileName, const NativePrefixFlagsInfo &npfi)
{
    if (npfi.netPrefix)
    {
        return getNativeNetworkPathPrefix<StringType>() + fileName;
    }

    if (npfi.networkUncPrefix)
    {
        return getNativeNetworkUncPrefix<StringType>() + fileName;
    }

    if (npfi.uncPrefix)
    {
        return getNativeUncPrefix<StringType>() + fileName;
    }

    return fileName;
}

// Добавляет нативные префиксы
template<typename StringType> inline
StringType addNativePrefixes(const StringType &fileName, const NativePrefixFlagsInfo &npfi, typename StringType::value_type pathSep)
{
    if (npfi.netPrefix)
    {
        return normalizePathSeparators(getNativeNetworkPathPrefix<StringType>(), pathSep) + fileName;
    }

    if (npfi.networkUncPrefix)
    {
        return normalizePathSeparators(getNativeNetworkUncPrefix<StringType>(), pathSep) + fileName;
    }

    if (npfi.uncPrefix)
    {
        return normalizePathSeparators(getNativeUncPrefix<StringType>(), pathSep) + fileName;
    }

    return fileName;
}

//----------------------------------------------------------------------------
//! Делает "каноническое" имя, схлопывая все лишние алиасы (".." и "."), и дублирующиеся разделители пути, не учитывая возможные спец префиксы
template<typename StringType> inline
std::vector< StringType > makeCanonicalSimpleParts( StringType fileName, typename StringType::value_type pathSep, const StringType &curDirAlias, const StringType &parentDirAlias, bool keepLeadingParents = false)
{
    namespace ustrp = umba::string_plus;

    std::vector< StringType > parts = ustrp::split(fileName, pathSep, true /* skipEmpty */ );
    std::vector< StringType > resParts; resParts.reserve(parts.size());

    std::size_t parentDirPrefixCounter = 0;

    typename std::vector< StringType >::iterator pit = parts.begin();
    for(; pit != parts.end(); ++pit)
    {
        if (*pit==curDirAlias)
            continue;

        if (*pit==parentDirAlias)
        {
            if (!resParts.empty())
            {
                resParts.erase( --resParts.end() );
            }
            else
            {
                ++parentDirPrefixCounter;
            }
            continue;
        }

        resParts.push_back(*pit);
    }

    if (parentDirPrefixCounter!=0 && keepLeadingParents)
    {
        std::vector< StringType > tmp = std::vector< StringType >(parentDirPrefixCounter, parentDirAlias);
        tmp.reserve(tmp.size()+resParts.size());
        tmp.insert(tmp.end(), resParts.begin(), resParts.end());
        resParts.swap(tmp);
    }

    return resParts;
}

//----------------------------------------------------------------------------
//! Делает "каноническое" имя, схлопывая все лишние алиасы (".." и "."), и дублирующиеся разделители пути, не учитывая возможные спец префиксы
template<typename StringType> inline
StringType makeCanonicalSimple( StringType fileName, typename StringType::value_type pathSep, const StringType &curDirAlias, const StringType &parentDirAlias, bool keepLeadingParents = false)
{
    namespace ustrp = umba::string_plus;

    fileName = normalizePathSeparators(fileName, pathSep);

    bool lastPathSep  = stripLastPathSep(fileName);
    bool firstPathSep = stripFirstPathSep(fileName);

    std::vector< StringType > parts = makeCanonicalSimpleParts( fileName, pathSep, curDirAlias, parentDirAlias, keepLeadingParents);

    fileName = ustrp::merge(parts,pathSep);

    if (firstPathSep)
    {
        fileName = StringType(1, pathSep) + fileName;
    }

    if (lastPathSep)
    {
        fileName.append(1, pathSep);
    }

    return fileName;
}

//----------------------------------------------------------------------------
//! Делает "каноническое" имя, схлопывая все лишние алиасы (".." и "."), и дублирующиеся разделители пути
template<typename StringType> inline
StringType makeCanonical( StringType fileName
                        , typename StringType::value_type pathSep
                        , const StringType &currentDirAlias
                        , const StringType &parentDirAlias
                        , bool keepLeadingParents
                        )
{
    // std::replace_if( fileName.begin(), fileName.end(), isPathSep<typename StringType::value_type>, pathSep );

    namespace ustrp = umba::string_plus;

    #if 0
    bool hasNetworkUncPrefix = false;
    bool hasUncPrefix = false;

    #if defined(WIN32) || defined(_WIN32)
    if (ustrp::starts_with_and_strip(fileName, getNativeNetworkUncPrefix<StringType>()))
        hasNetworkUncPrefix = true;
    else if (ustrp::starts_with_and_strip(fileName, getNativeUncPrefix<StringType>()))
        hasUncPrefix = true;
    #endif

    bool hasNetPrefix = false;
    if (ustrp::starts_with_and_strip(fileName, getNativeNetworkPathPrefix<StringType>()))
        hasNetPrefix = true;
    #endif

    // Сначала обрезаем нативные префиксы, если есть
    //std::replace_if( fileName.begin(), fileName.end(), isPathSep<typename StringType::value_type>, pathSep );

    NativePrefixFlagsInfo npfi = stripNativePrefixes(fileName, pathSep);

    fileName = normalizePathSeparators(fileName, pathSep);

    bool lastPathSep  = stripLastPathSep(fileName);
    bool firstPathSep = stripFirstPathSep(fileName);

    #if 0
    bool lastPathSep = stripLastPathSep(fileName);

    std::vector< StringType > parts = ustrp::split(fileName, pathSep, true /* skipEmpty */ );

    std::vector< StringType > resParts;

    StringType nativeCurrentDirAlias = getNativeCurrentDirAlias<StringType>();
    StringType nativeParentDirAlias  = getNativeParentDirAlias<StringType>();

    typename std::vector< StringType >::iterator pit = parts.begin();
    for(; pit != parts.end(); ++pit)
    {
        if (*pit==nativeCurrentDirAlias)
            continue;

        if (*pit==nativeParentDirAlias)
        {
            if (!resParts.empty())
                resParts.erase( --resParts.end() );
            continue;
        }

        resParts.push_back(*pit);
    }
    #endif

    std::vector< StringType > parts = makeCanonicalSimpleParts( fileName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents);

    typename std::vector< StringType >::iterator pit = parts.begin();
    StringType nativeHomeDirAlias = getNativeHomeDirAlias<StringType>();
    if (!npfi.hasAnyPrefix() /* !hasNetPrefix && !hasNetworkUncPrefix && !hasUncPrefix */ && pit!=parts.end() && !nativeHomeDirAlias.empty() && *pit==nativeHomeDirAlias)
    {
        StringType homePath = filesys::internal::getCurrentUserHomeDirectory<StringType>();
        if (!homePath.empty())
        {
            std::replace_if( homePath.begin(), homePath.end(), isPathSep<typename StringType::value_type>, pathSep );
            std::vector< StringType > homeParts = ustrp::split(homePath, pathSep, true /* skipEmpty */ );

            parts.erase(pit);
            parts.insert( parts.begin(), homeParts.begin(), homeParts.end() );
        }
    }

    fileName = ustrp::merge(parts,pathSep);

    if (firstPathSep)
    {
        fileName = StringType(1, pathSep) + fileName;
    }

    if (lastPathSep)
    {
        fileName.append(1, pathSep);
    }


    #if 0
    if (lastPathSep)
        appendPathSepInline(res,pathSep);

    if (hasNetPrefix)
        res = getNativeNetworkPathPrefix<StringType>() + res;

    if (hasNetworkUncPrefix)
        res = getNativeNetworkUncPrefix<StringType>() + res;
    else if (hasUncPrefix)
        res = getNativeUncPrefix<StringType>() + res;

    return res;
    #endif

    return addNativePrefixes(fileName, npfi, pathSep);
}

//-----------------------------------------------------------------------------
//! Делает "каноническое" имя для сравнения
template<typename StringType> inline
StringType makeCanonicalForCompare( StringType                      fileName
                                  , typename StringType::value_type pathSep            = getNativePathSep<typename StringType::value_type>()
                                  , const StringType                &currentDirAlias   = umba::filename::getNativeCurrentDirAlias<StringType>()
                                  , const StringType                &parentDirAlias    = umba::filename::getNativeParentDirAlias<StringType>()
                                  , bool                            keepLeadingParents = false
                                  )
{
    #if defined(WIN32) || defined(_WIN32)
    // namespace ustrp = umba::string_plus;
    //
    // NativePrefixFlagsInfo npfi = stripNativePrefixes(fileName, pathSep);
    // // npfi.networkUncPrefix      = false;
    // // npfi.uncPrefix             = false;
    // // fileName                   = addNativePrefixes(fileName, npfi, pathSep);

    // StringType canoname        = makeCanonical(fileName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents);
    // canoname = ustrp::tolower_copy(canoname);
    //
    // return addNativePrefixes(canoname, npfi, pathSep);

    return umba::string_plus::tolower_copy(makeCanonical(fileName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents));

    #if 0
    if (ustrp::starts_with_and_strip(canoname, getNativeNetworkUncPrefix<StringType>()))
        canoname = getNativeNetworkPathPrefix<StringType>() + canoname;
    else
        ustrp::starts_with_and_strip(canoname, getNativeUncPrefix<StringType>());
    #endif

    //return ustrp::toupper_copy(canoname);
    //return ustrp::tolower_copy(canoname);
    //return canoname; // Почему регистр не меняем, я хз, и почему раньше меняли, а сейчас - нет - хз
    #else
    return makeCanonical(fileName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents);
    #endif
}

//-----------------------------------------------------------------------------
#include "umba/warnings/push_disable_spectre_mitigation.h"
//! Возвращает true,  если путь абсолютный
template<typename StringType> inline
bool isAbsPath( StringType p, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
{
    namespace ustrp = umba::string_plus;

    p = makeCanonical(p);

    #if defined(WIN32) || defined(_WIN32)
    if (ustrp::starts_with(p, getNativeUncPrefix<StringType>()))
        return true;
    #endif

    if (ustrp::starts_with(p, ustrp::make_string<StringType>(pathSep)))
        return true;

    #if defined(WIN32) || defined(_WIN32)
    if (p.size()<2)
        return false;
    if ( (ustrp::is_alpha(p[0]) && (p[1]==(typename StringType::value_type)':')) || ((p.size()>2 && ustrp::is_alpha(p[1]) && p[2]==(typename StringType::value_type)':')) )
        return true;
    #endif

    return false;
}
#include "umba/warnings/pop.h"

//-----------------------------------------------------------------------------
//! Подготавливает имя для "нативного" использования - для передачи имени в системные API
template<typename StringType> inline
StringType prepareForNativeUsage( const StringType &fileName )
{

    #if defined(WIN32) || defined(_WIN32)

        if constexpr (sizeof(typename StringType::value_type)==sizeof(char))
        {
            return makeCanonical(fileName);
        }
        else
        {
            if (!isAbsPath(fileName))
            {
                return makeCanonical(fileName);
            }

            // А надо ли вообще тут делать makeCanonical?
            // StringType canoname = fileName; //makeCanonical(fileName);
            StringType canoname = makeCanonical(fileName);

            namespace ustrp = umba::string_plus;

            //if (hasNativeNetworkPathPrefix(canoname))
            //{
            //    return canoname;
            //}
            if (ustrp::starts_with(canoname, getNativeNetworkUncPrefix<StringType>()))
                return canoname; // already UNC name

            StringType nativeUncPrefix = getNativeUncPrefix<StringType>();
            if (ustrp::starts_with(canoname, nativeUncPrefix))
                return canoname; // already UNC name

            if (ustrp::starts_with_and_strip(canoname, getNativeNetworkPathPrefix<StringType>()))
            {
                return getNativeNetworkUncPrefix<StringType>() + canoname;
            }

            return getNativeUncPrefix<StringType>() + canoname;
        }

    #else

        return makeCanonical(fileName);

    #endif
}

//-----------------------------------------------------------------------------
//! Делает абсолютный путь под CWD, если задан относительный path
template<typename StringType> inline
StringType makeAbsPath( const StringType &path
                      , const StringType &cwd  = umba::filesys::internal::getCurrentDirectory<StringType>()
                      , typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>()
                      )
{
    if (isAbsPath(path, pathSep))
        return path;

    return appendPath(cwd, path, pathSep);
}

//-----------------------------------------------------------------------------
//! Удаляет префикс пути - делает имя относительным
template<typename StringType> inline
bool isSubPathName( const StringType                commonPath_
                  , const StringType                fullName_
                  , StringType                      *pResName          = 0
                  , typename StringType::value_type pathSep            = getNativePathSep<typename StringType::value_type>()
                  , const StringType                &currentDirAlias   = umba::filename::getNativeCurrentDirAlias<StringType>()
                  , const StringType                &parentDirAlias    = umba::filename::getNativeParentDirAlias<StringType>()
                  , bool                            keepLeadingParents = false
                  )
{
    auto commonPath = makeCanonicalForCompare(commonPath_, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents);
    auto fullName   = makeCanonicalForCompare(fullName_  , pathSep, currentDirAlias, parentDirAlias, keepLeadingParents);

    appendPathSepInline(commonPath, pathSep);
    //if (umba::string_plus::starts_with_and_strip(fullName, commonPath))
    if (umba::string_plus::starts_with(fullName, commonPath))
    {
        auto orgFullName = makeCanonical(fullName_, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents); // Отличие в том, что регистр не меняется
        orgFullName.erase(0, commonPath.size());

        if (pResName)
           *pResName = orgFullName;

        return true;
    }

    return false;
}

    // StringType makeCanonical( StringType fileName
    //                         , typename StringType::value_type pathSep  = umba::filename::getNativePathSep<typename StringType::value_type>()
    //                         , const StringType &currentDirAlias        = umba::filename::getNativeCurrentDirAlias<StringType>()
    //                         , const StringType &parentDirAlias         = umba::filename::getNativeParentDirAlias<StringType>()
    //                         , bool keepLeadingParents                  = false
    //                         );

//-----------------------------------------------------------------------------
//! Удаляет префикс пути - делает имя относительным
template<typename StringType> inline
bool makeRelPath( StringType                      &foundRelName
                , const StringType                &commonPath
                , const StringType                &fullName
                , typename StringType::value_type pathSep            = getNativePathSep<typename StringType::value_type>()
                , const StringType                &currentDirAlias   = umba::filename::getNativeCurrentDirAlias<StringType>()
                , const StringType                &parentDirAlias    = umba::filename::getNativeParentDirAlias<StringType>()
                , bool                            keepLeadingParents = false
                , bool                            tryReverseRelPath   = false
                )
{
    StringType res;
    if (isSubPathName(commonPath, fullName, &res, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents))
    {
        foundRelName = res;
        return true;
    }

    if (!tryReverseRelPath)
        return false;

    StringType fullNamePath = getPath(fullName); // относительное имя ищем для последнего компонента пути, "отрезаем" его тут

    if (umba::filename::isSubPathName(fullNamePath, commonPath, &res))
    {
        res = umba::filename::normalizePathSeparators(res, pathSep);
        std::vector<std::string> parts = umba::string_plus::split(res, pathSep, true /* skipEmpty */ );
        //res = umba::filename::appendPath(umba::string_plus::merge(std::vector<std::string>(parts.size(), ".."), '/'), umba::filename::getFileName(url));
        res = appendPath( umba::string_plus::merge(std::vector<std::string>(parts.size(), parentDirAlias), pathSep)
                        , getFileName(fullName)
                        , pathSep
                        );
        foundRelName = res;
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//! Удаляет префикс пути - делает имя относительным
template<typename StringType> inline
StringType makeRelPath( const StringType                &fullName
                      , const StringType                &commonPath
                      , typename StringType::value_type pathSep            = getNativePathSep<typename StringType::value_type>()
                      , const StringType                &currentDirAlias   = umba::filename::getNativeCurrentDirAlias<StringType>()
                      , const StringType                &parentDirAlias    = umba::filename::getNativeParentDirAlias<StringType>()
                      , bool                            keepLeadingParents = false
                      , bool                            tryReverseRelPath  = false
                      )
{
    StringType res;
    if (makeRelPath(res, commonPath, fullName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents, tryReverseRelPath))
        return res;

    return fullName;
}

//-----------------------------------------------------------------------------
//! Удаляет префикс пути - делает имя относительным
template<typename StringType> inline
bool makeRelPath( StringType                      &foundRelName
                , const std::vector<StringType>   &commonPaths
                , const StringType                &fullName
                , typename StringType::value_type pathSep            = getNativePathSep<typename StringType::value_type>()
                , const StringType                &currentDirAlias   = umba::filename::getNativeCurrentDirAlias<StringType>()
                , const StringType                &parentDirAlias    = umba::filename::getNativeParentDirAlias<StringType>()
                , bool                            keepLeadingParents = false
                , bool                            tryReverseRelPath  = false
                )
{
    for(const StringType &commonPath : commonPaths)
    {
        if (makeRelPath(foundRelName, commonPath, fullName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents, false))
            return true;
    }

    if (!tryReverseRelPath)
        return false;

    for(const StringType &commonPath : commonPaths)
    {
        if (makeRelPath(foundRelName, commonPath, fullName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents, true))
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//! Удаляет префикс пути - делает имя относительным
template<typename StringType> inline
StringType makeRelPath( const std::vector<StringType>   &commonPaths
                      , const StringType                &fullName
                      , typename StringType::value_type pathSep            = getNativePathSep<typename StringType::value_type>()
                      , const StringType                &currentDirAlias   = umba::filename::getNativeCurrentDirAlias<StringType>()
                      , const StringType                &parentDirAlias    = umba::filename::getNativeParentDirAlias<StringType>()
                      , bool                            keepLeadingParents = false
                      , bool                            tryReverseRelPath  = false
                      )
{
    StringType res;
    if (makeRelPath(res, commonPaths, fullName, pathSep, currentDirAlias, parentDirAlias, keepLeadingParents, tryReverseRelPath))
        return res;

    return fullName;
}

//-----------------------------------------------------------------------------
//! Добавляет путь (или имя файла) к другому пути
template<typename StringType> inline StringType appendPath( const StringType &p, const StringType &f, typename StringType::value_type pathSep )
{
    if (p.empty())
        return f;

    if (f.empty())
        return p;

    if (hasLastPathSep(p))
        return p + stripFirstPathSepCopy(f);

    if (hasFirstPathSep(f))
        return p + f;

    return p + StringType(1, pathSep) + f;
}

//-----------------------------------------------------------------------------
template<typename StringType> inline StringType appendPath( const StringType &p, const StringType &f )
{
    return appendPath(p,f,getNativePathSep<typename StringType::value_type>());
}

//-----------------------------------------------------------------------------
template<typename StringType> inline StringType appendPath( const StringType &p, const typename StringType::value_type *f_, typename StringType::value_type pathSep )
{
    StringType f = f_;

    if (p.empty())
        return f;

    if (f.empty())
        return p;

    if (hasLastPathSep(p))
        return p + stripFirstPathSepCopy(f);

    if (hasFirstPathSep(f))
        return p + f;

    return p + StringType(1, pathSep) + f;
}

//-----------------------------------------------------------------------------
template<typename StringType> inline StringType appendPath( const StringType &p, const typename StringType::value_type *f )
{
    return appendPath(p,f,getNativePathSep<typename StringType::value_type>());
}

//-----------------------------------------------------------------------------
//! Добавляет расширение
template<typename StringType> inline
StringType appendExt( const StringType &n, const StringType &e, typename StringType::value_type extSep = getNativeExtSep<typename StringType::value_type>() )
{

    // Если расширение не задано вообще, то не приклеиваем его, и не приклеиваем даже точку/разделитель расширения
    if (e.empty())
        return n;

    // Если в качестве расширения задана только точка, то она будет приклеена к имени файла

/*
    if (n.empty())
        return e;

    if (e.empty())
        return n;

    if (hasLastExtSep(n))
        return n + stripFirstExtSepCopy(e);

    if (hasFirstExtSep(e))
        return n + e;

    return n + StringType(1, extSep) + e;
*/
    return stripLastExtSepCopy(n) + StringType(1, extSep) + stripFirstExtSepCopy(e);
}

//-----------------------------------------------------------------------------
//! Добавляет расширение
template<typename StringType> inline
StringType appendExtention( const StringType &n, const StringType &e, typename StringType::value_type extSep = getNativeExtSep<typename StringType::value_type>() )
{
    return appendExt(n, e, extSep);
}
//inline CharType getNativeExtSep( )

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
enum class PathPrefixType : unsigned
{
    unknown, undefined      = (unsigned)-1,

    none                    = 0,

    flagWin32               = 0x0100, // Win32 specific prefix
    flagNetwork             = 0x0200, // признак сетевого пути

    win32FileNamespace      = 0x0101, // \\?\ (Win32 File Namespace), Extended-Length Path Prefix - применяется для обхода стандартного ограничения Windows в 260 символов
    win32DeviceNamespace    = 0x0102, // \\.\ (Win32 Device Namespace) - Используется для прямого доступа к физическим устройствам (например, COM1 или PhysicalDrive0) в обход файловой системы

    networkPath             = 0x0203, // сетевой путь "//" ("\\\\")
    uncNetPath              = 0x0304, // сетевой путь в Windows вида \\?\UNC\server\share (flagWin32|flagNetwork)

};

//-----------------------------------------------------------------------------
inline
bool isPathPrefixTypeWin32(PathPrefixType ppt)
{
    return ((unsigned)ppt & (unsigned)PathPrefixType::flagWin32) != 0;
}

//-----------------------------------------------------------------------------
inline
bool isPathPrefixTypeNetwork(PathPrefixType ppt)
{
    return ((unsigned)ppt & (unsigned)PathPrefixType::flagNetwork) != 0;
}

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
template<typename StringType> inline
void findAllFullPathComponentSeparators( const StringType &p
                                       , typename StringType::size_type *pDriveSepPos       = 0 // Возвращается первый разделитель буквы диска
                                       , typename StringType::size_type *pPathSepPos        = 0 // Возвращается последний разделитель пути
                                       , typename StringType::size_type *pExtSepPos         = 0 // Возвращается последний разделитель расширения (только в последнем компоненте пути)
                                       , typename StringType::size_type *pAdsSepPos         = 0 // Возвращается разделитель ADS
                                       , typename StringType::size_type *pPathStartPos      = 0 // Возвращается позиция начала пути, перед ней либо сетевой префикс с именем сервера и шары, либо специальный виндовый префикс
                                       , PathPrefixType                 *pPathPrefixType    = 0 // Возвращается тип префикса пути, если таковой есть
                                       )
{
    // Разделитель пути - поддерживается как '\', так и '/' - нет разницы, функция isPathSep
    // Разделитель расширения - обычно это точка - Windows/Linux, функция isExtSep
    // Разделитель диска - только Windows - двоеточие ':', функция isDriveSep
    // Разделитель ads - только Windows - Alternate Data Streams, ADS - тоже двоеточие ':', функция isAdsSep

    // Функции isAdsSep и isDriveSep могут возвращать true, если в системе есть понятие дисков и потоков ADS.
    // Функция isPathSep детектирует разделитель пути как Windows, так и Linux. 
    // isExtSep - реагирует на символ точка - это универсальный разделитель расширения.

    // Важно: имя файла, которое начинается с точки мы считаем, что это имя имеет только расширение, а само имя пустое
    // Имена вида ".tar.gz": это пустое имя файла с двойным расширением, при первом получении имени файла без расширения
    // будет возвращено ".tar", при втором получении имени файла без расширения будет возвращена пустая строка.

    // Имя вида "C:file.txt", вообще говоря, некорректно - отсутствует разделитель пути после двоеточия,
    // но мы хотим его корректно обрабатывать как диск "C:" и файл "file.txt" без пути.

    // StringType getNativeUncPrefix()         // Возвращает под Windows "\\\\?\\", иначе - пустую строку (на самом деле правильно называть этот префикс Extended-Length Path или Win32 File Namespace)
    // StringType getNativeNetworkUncPrefix()  // Возвращает сетевой UNC префикс - под Windows "\\\\?\\UNC\\", иначе - пустую строку
    // StringType getNativeNetworkPathPrefix() // Возвращает сетевой префикс - "\\\\" под Windows, иначе "//" 

    // Пролезная ссылка: The Definitive Guide on Win32 to NT Path Conversion - https://projectzero.google/2016/02/the-definitive-guide-on-win32-to-nt.html

    // AI review - https://chat.deepseek.com/share/pu04zk455gw6miz84x

    using CharType = typename StringType::value_type;
    using SizeType = typename StringType::size_type;

    SizeType driveSepPos = p.npos;
    SizeType pathSepPos  = p.npos;
    SizeType extSepPos   = p.npos;
    SizeType adsSepPos   = p.npos;

    SizeType pathStartPos = 0;

    SizeType pos = 0;

    PathPrefixType pathPrefixType = PathPrefixType::none;


    // Копирование строки с нормализацией разделителя пути и регистра символов
    // выглядит более предпочтительным, чем при каждом сравнении учитывать регистр символов
    // разделители пути
    auto simpleConvertToCompatName = [](StringType path)
    {
        for(auto &ch : path)
        {
            if (ch==(CharType)'\\')
            {
                ch = (CharType)'/';
            }
        }

        // string::toupper меняет регистр символов (только латинских) inplace.
        // Тут всё корректно, даже если локали не поддерживаются - нас интересуют только возможные префиксы, а они только в пределах базовой ASCII таблицы.
        // Даже если toupper накосячит с кодировками, в префиксах используется только латиница, и всё будет нормально
        // В дальнейшем же мы используем оригинальную строку
        string::toupper(path); 

        return path;
    };

    // bool isNetworkPath   = false;
    // bool isSpecialPrefix = false;

    // flagWin32               = 0x0100, // Win32 specific prefix
    // flagNetwork             = 0x0200, // признак сетевого пути
    //  
    // win32FileNamespace      = 0x0101, // \\?\ (Win32 File Namespace), Extended-Length Path Prefix - применяется для обхода стандартного ограничения Windows в 260 символов
    // win32DeviceNamespace    = 0x0102, // \\.\ (Win32 Device Namespace) - Используется для прямого доступа к физическим устройствам (например, COM1 или PhysicalDrive0) в обход файловой системы
    //  
    // networkPath             = 0x0203, // сетевой путь "//" ("\\\\")
    // uncNetPath              = 0x0304, // сетевой путь в Windows вида \\?\UNC\server\share (flagWin32|flagNetwork)


    {
        static const auto uncPrefix         = simpleConvertToCompatName(getNativeUncPrefix<StringType>());          // средний             под Windows "\\\\?\\"
        static const auto networkUncPrefix  = simpleConvertToCompatName(getNativeNetworkUncPrefix<StringType>());   // самый длинный       под Windows "\\\\?\\UNC\\"
        static const auto networkPrefix     = simpleConvertToCompatName(getNativeNetworkPathPrefix<StringType>());  // самый короткий      "\\\\" под Windows, иначе "//"

#if defined(WIN32) || defined(_WIN32)
        static const auto devicePrefix      = simpleConvertToCompatName(string::make_string<StringType>("\\\\.\\")); // такой же по длине, как uncPrefix, под Windows "\\\\.\\"
        // static const auto uncSimple         = simpleConvertToCompatName(string::make_string<StringType>("UNC\\"));
#else
        static const auto devicePrefix      = StringType();
        // static const auto uncSimple         = StringType();
#endif

        auto pathTmp = simpleConvertToCompatName(p);
        SizeType specialPrefixLen = 0;

        if (!networkUncPrefix.empty() && string::starts_with(pathTmp, networkUncPrefix)) // "\\\\?\\UNC\\"
        {
            specialPrefixLen = networkUncPrefix.size();
            pathPrefixType   = PathPrefixType::uncNetPath;
        }

        else if (!uncPrefix.empty() && string::starts_with(pathTmp, uncPrefix)) // "\\\\?\\"
        {
            specialPrefixLen = uncPrefix.size();
            pathPrefixType   = PathPrefixType::win32FileNamespace;
        }

        else if (!devicePrefix.empty() && string::starts_with(pathTmp, devicePrefix)) // "\\\\.\\"
        {
            specialPrefixLen = devicePrefix.size();
            pathPrefixType   = PathPrefixType::win32DeviceNamespace;

            // Вроде бы это был загон deepseek'а, реально такой фичи не существует
            // if (!uncSimple.empty() && string::starts_with(StringType(pathTmp, specialPrefixLen, pathTmp.npos), uncSimple))
            // {
            //     // специальный видновый префикс для сетевой шары
            //     specialPrefixLen += uncSimple.size();
            //     isNetworkPath = true;
            // }
        }

        else if (!networkPrefix.empty() && string::starts_with(pathTmp, networkPrefix)) // "//"
        {
            specialPrefixLen = networkPrefix.size();
            pathPrefixType   = PathPrefixType::networkPath;
        }

        if (specialPrefixLen!=0)
        {
            pathStartPos = specialPrefixLen;

            if (isPathPrefixTypeNetwork(pathPrefixType))
            {
                // Пропускаем имя сервера
                for(; pathStartPos!=p.size(); ++pathStartPos)
                {
                    if (isPathSep(p[pathStartPos]))
                        break;
                }

                if (pathStartPos!=p.size())
                    ++pathStartPos;
                // else
                //     isNetworkPath = false; // что-то непонятное - точно отсутствует имя шары в сетевом пути - но ничего не делаем, это всё равно сетевой путь, который указывает только на сервер

                // Пропускаем имя шары
                for(; pathStartPos!=p.size(); ++pathStartPos)
                {
                    if (isPathSep(p[pathStartPos]))
                        break;
                }

                // IMPORTANT! Тут мы не пропускаем разделитель пути пусле имени шары - если после имени шары есть путь,
                // то он будет выглядеть как абсолютный путь Linux или как абсолютный путь Windows для текущего диска.
                // Если же пропускать этот разделитель, то путь (без сетевого или специального префикса) будет выглядеть, как относительный.

            } // if (isNetPrefix)

        } // if (specialPrefixLen!=0)

        pos = pathStartPos;

        if (pPathStartPos)
           *pPathStartPos = pathStartPos;

        // if (pIsNetworkPath)
        //    *pIsNetworkPath = isNetworkPath;
        //  
        // if (pHasSpecialPrefix)
        //    *pHasSpecialPrefix = isSpecialPrefix;

        if (pPathPrefixType)
           *pPathPrefixType = pathPrefixType;

    }


    bool hasOnlyAlphas = true; // флаг, сигнализирующий о том, то в имени пути до текущего момента встречались только латинские буквы
    SizeType pathPartLen = 0;
    SizeType pathComponentCount = 0;

    for(; pos!=p.size(); ++pos)
    {
        auto ch = p[pos];

        if (isPathSep(ch))
        {
            pathSepPos = pos;
            // if (pathSepPosF==p.npos)
            //     pathSepPosF = pos; // первый разделитель пути нашли

            adsSepPos = p.npos; // сбрасываем ads разделитель - он не может быть до разделителя пути

            extSepPos = p.npos; // сбрасываем разделитель расширения

            hasOnlyAlphas = true; // восстанавливаем флаг для следующего компонента пути

            pathPartLen = 0; // сбрасываем длину текущей части пути

            ++pathComponentCount;

            continue;
        }

        if (isExtSep(ch))
        {
            if (adsSepPos!=p.npos) // у нас уже есть ads разделитель
            {
                // Ничего не делаем - не детектим разделитель расширения в имени ads, только в имени файла
            }
            else // у нас нет ads - это обычное расширение имени файла
            {
                extSepPos = pos;
            }

            continue;
        }

        // Имя диска в системах с большим количеством дисков может быть не из одной латинской буквы, но это не точно.
        // Я вроде видел двухбуквенные диски в реальном использовании, но не факт
        // Пока ограничимся однобуквенными дисками
        // Разделитель диска проверяем только в первом компоненте пути, до всех слешей. Это корректно будет работать как для пути без префиксов, 
        // так и для пути с префиксом \\?\ в Windows
        if (!isPathPrefixTypeNetwork(pathPrefixType) && pathComponentCount==0 && pathPartLen==1 && hasOnlyAlphas && driveSepPos==p.npos && isDriveSep(ch)) // заменяем только если этого символа ещё не было
        {
            driveSepPos = pos;
        }

        if (driveSepPos!=pos && adsSepPos==p.npos && isAdsSep(ch)) // Детектим только первый разделитель ADS и если разделитель диска не равен текущей позиции
        {
            adsSepPos = pos;
        }

        if (!((ch>='a' && ch<='z') || (ch>='A' && ch<='Z')))
            hasOnlyAlphas = false;

        ++pathPartLen;

    } // for

    // ads мы сбрасываем при обнаружении разделителя пути
    if (adsSepPos!=p.npos && adsSepPos==driveSepPos) // наден adsSep
    {
        // найден и driveSep и adsSep, и они равны
        // это может быть adsSep - но тогда путь без диска - сетевой или относительный
        if (extSepPos!=p.npos && extSepPos<adsSepPos)
            driveSepPos = p.npos; // если у нас есть разделитель расширения, и он идёт раньше ads разделителя, то это точно не разделитель диска - сбрасываем разделитель диска
        else 
            adsSepPos = p.npos; // разделителя расширения нет, или разделитель расширения позже ads разделителя, то это не ads разделитель, а разделитель диска - сбрасываем ads
    }

    // if (pathSepPos==p.npos && pathStartPos!=0)
    //     pathSepPos = pathStartPos;

    if (pDriveSepPos) *pDriveSepPos = driveSepPos;
    if (pPathSepPos ) *pPathSepPos  = pathSepPos ;
    if (pExtSepPos  ) *pExtSepPos   = extSepPos  ;
    if (pAdsSepPos  ) *pAdsSepPos   = adsSepPos  ;
}

//-----------------------------------------------------------------------------
//! Извлекает путь из имени без последнего разделителя пути
template<typename StringType> inline
StringType getPath( const StringType &s )
{
    typename StringType::size_type driveSepPos  = s.npos;
    typename StringType::size_type pathSepPos   = s.npos;
    //typename StringType::size_type pathStartPos = 0;

    //findAllFullPathComponentSeparators(s, &driveSepPos, &pathSepPos, 0, 0, &pathStartPos);
    findAllFullPathComponentSeparators(s, &driveSepPos, &pathSepPos);

    if (pathSepPos!=s.npos)
    {
        // Удаляем все хвостовые 
        auto res = StringType(s, 0, pathSepPos);
        while(!res.empty() && isPathSep(res[res.size()-1]))
             res.erase(res.size()-1, 1);
        return res;
    }

    if (driveSepPos!=s.npos)
    {
        return StringType(s, 0, driveSepPos+1);
    }

    return s;

}

// void findAllFullPathComponentSeparators( const StringType &p
//                                        , typename StringType::size_type *pDriveSepPos       = 0 // Возвращается первый разделитель буквы диска
//                                        , typename StringType::size_type *pPathSepPos        = 0 // Возвращается последний разделитель пути
//                                        , typename StringType::size_type *pExtSepPos         = 0 // Возвращается последний разделитель расширения (только в последнем компоненте пути)
//                                        , typename StringType::size_type *pAdsSepPos         = 0 // Возвращается разделитель ADS
//                                        , typename StringType::size_type *pPathStartPos      = 0 // Возвращается позиция начала пути, перед ней либо сетевой префикс с именем сервера и шары, либо специальный виндовый префикс
//                                        , PathPrefixType                 *pPathPrefixType    = 0 // Возвращается тип префикса пути, если таковой есть
//                                        )


inline std::string  getPath( const char    *p ) { return getPath<std::string> ( p ); } //!< Извлекает путь из имени
inline std::wstring getPath( const wchar_t *p ) { return getPath<std::wstring>( p ); } //!< Извлекает путь из имени

//-----------------------------------------------------------------------------
//! Извлекает из полного пути имя файла + расширение
template<typename StringType> inline
StringType getFileName( const StringType &s )
{
    typename StringType::size_type driveSepPos  = s.npos;
    typename StringType::size_type pathSepPos   = s.npos;
    typename StringType::size_type adsSepPos    = s.npos;
    typename StringType::size_type pathStartPos = s.npos;

    findAllFullPathComponentSeparators(s, &driveSepPos, &pathSepPos, 0, &adsSepPos, &pathStartPos);

    typename StringType::size_type startPos = 0;
    //typename StringType::size_type len      = s.size();

    if (pathSepPos!=s.npos)
        startPos = pathSepPos+1;
    else if (driveSepPos!=s.npos)
        startPos = driveSepPos+1;

    if (pathStartPos!=s.npos && startPos<pathStartPos)
        startPos = pathStartPos;

    typename StringType::size_type endPos = s.size();
    if (adsSepPos != s.npos)
        endPos = adsSepPos;

    return StringType(s, startPos, endPos - startPos);
}

inline std::string  getFileName( const char    *p ) { return getFileName<std::string> ( p ); } //!< Извлекает из полного пути имя файла + расширение
inline std::wstring getFileName( const wchar_t *p ) { return getFileName<std::wstring>( p ); } //!< Извлекает из полного пути имя файла + расширение

//-----------------------------------------------------------------------------
//! Извлекает из полного пути путь и имя файла без расширения
template<typename StringType> inline
StringType getPathFile( const StringType &path )
{
    typename StringType::size_type extSepPos    = path.npos;
    typename StringType::size_type adsSepPos    = path.npos;
    typename StringType::size_type pathStartPos = path.npos;

    findAllFullPathComponentSeparators(path, 0, 0, &extSepPos, &adsSepPos, &pathStartPos);

    typename StringType::size_type endPos = path.size();
    typename StringType::size_type startPos = 0;

    if (adsSepPos != path.npos)
        endPos = adsSepPos;

    if (extSepPos!=path.npos)
        endPos = extSepPos;

    return StringType(path, 0, endPos - startPos);
}

inline std::string  getPathFile( const char    *p ) { return getPathFile<std::string> ( p ); } //!< Извлекает из полного пути имя файла + расширение
inline std::wstring getPathFile( const wchar_t *p ) { return getPathFile<std::wstring>( p ); } //!< Извлекает из полного пути имя файла + расширение

//-----------------------------------------------------------------------------
//! Извлекает из имени расширение
template<typename StringType> inline
StringType getFileExtention( const StringType &path )
{
    typename StringType::size_type extSepPos   = path.npos;
    typename StringType::size_type adsSepPos   = path.npos;

    findAllFullPathComponentSeparators(path, 0, 0, &extSepPos, &adsSepPos);

    if (extSepPos==path.npos)
        return StringType();

    typename StringType::size_type startPos = extSepPos+1;

    if (adsSepPos==path.npos)
        adsSepPos = path.size();

    typename StringType::size_type len = adsSepPos - startPos;

    return StringType(path, startPos, len);
}

//! Извлекает из имени расширение
template<typename StringType> inline
StringType getFileExt( const StringType &path )
{
    return getFileExtention<StringType>(path);
}

inline std::string  getFileExtention( const char    *p ) { return getFileExtention<std::string> ( p ); } //!< Извлекает из имени расширение
inline std::wstring getFileExtention( const wchar_t *p ) { return getFileExtention<std::wstring>( p ); } //!< Извлекает из имени расширение

inline std::string  getFileExt      ( const char    *p ) { return getFileExtention<std::string> ( p ); } //!< Извлекает из имени расширение
inline std::wstring getFileExt      ( const wchar_t *p ) { return getFileExtention<std::wstring>( p ); } //!< Извлекает из имени расширение

//! Извлекает из имени расширение - для совместимости
template<typename StringType> inline
StringType getExt( const StringType &path )
{
    return getFileExtention<StringType>(path);
}

inline std::string  getExt( const char    *p ) { return getFileExtention<std::string> ( p ); } //!< Извлекает из имени расширение
inline std::wstring getExt( const wchar_t *p ) { return getFileExtention<std::wstring>( p ); } //!< Извлекает из имени расширение


//-----------------------------------------------------------------------------

// void findAllFullPathComponentSeparators( const StringType &p
//                                        , typename StringType::size_type *pDriveSepPos       = 0 // Возвращается первый разделитель буквы диска
//                                        , typename StringType::size_type *pPathSepPos        = 0 // Возвращается последний разделитель пути
//                                        , typename StringType::size_type *pExtSepPos         = 0 // Возвращается последний разделитель расширения (только в последнем компоненте пути)
//                                        , typename StringType::size_type *pAdsSepPos         = 0 // Возвращается разделитель ADS




//! Извлекает из имени имя файла без пути и расширения
template<typename StringType> inline
StringType getName( const StringType &path )
{
    typename StringType::size_type driveSepPos  = path.npos;
    typename StringType::size_type pathSepPos   = path.npos;
    typename StringType::size_type extSepPos    = path.npos;
    typename StringType::size_type adsSepPos    = path.npos;
    typename StringType::size_type pathStartPos = path.npos;

    findAllFullPathComponentSeparators(path, &driveSepPos, &pathSepPos, &extSepPos, &adsSepPos, &pathStartPos);

    typename StringType::size_type startPos = 0;
    typename StringType::size_type endPos   = path.size();

    if (pathSepPos!=path.npos)
        startPos = pathSepPos+1;
    else if (driveSepPos!=path.npos)
        startPos = driveSepPos+1;

    if (pathStartPos!= path.npos && startPos<pathStartPos)
        startPos = pathStartPos;

    if (adsSepPos != path.npos)
        endPos = adsSepPos;

    if (extSepPos != path.npos)
        endPos = extSepPos;
     
    return StringType(path, startPos, endPos - startPos);
}

inline std::string  getName( const char    *p ) { return getName<std::string> ( p ); } //!< Извлекает из имени имя файла без пути и расширения
inline std::wstring getName( const wchar_t *p ) { return getName<std::wstring>( p ); } //!< Извлекает из имени имя файла без пути и расширения


//-----------------------------------------------------------------------------
//! Извлекает имя диска из пути
template<typename StringType> inline
StringType getDrive( StringType n )
{
    #if defined(WIN32) || defined(_WIN32)
    namespace ustrp = umba::string_plus;
    n = stripNativeUncPrefix( n );
    n = makeCanonical( n );
    std::vector< StringType > parts = ustrp::split(n, getNativePathSep<typename StringType::value_type>(), true /* skipEmpty */ );
    if (parts.empty())
        return StringType();

    const StringType &p1 = parts[0];
    if (p1.empty())
        return StringType();

    if (p1[p1.size()-1]==(typename StringType::value_type)':')
        return p1;

    #endif

    return StringType();

}

//-----------------------------------------------------------------------------
//! Проверяет путь на присутствие имени диска, и если отсутствует, то обновляет его, извлекая имя диска из другого пути
template<typename StringType> inline
StringType checkPathPrependDrive( const StringType &n, const StringType &curDir = filesys::internal::getCurrentDirectory<StringType>())
{
    #if defined(WIN32) || defined(_WIN32)
    namespace ustrp = umba::string_plus;

    StringType tmpCaninical = makeCanonical<StringType>(n);
    if ( ustrp::starts_with(tmpCaninical, getNativeUncPrefix<StringType>())
      || ustrp::starts_with_and_strip(tmpCaninical, getNativeNetworkPathPrefix<StringType>())
       )
        return n;

    if (hasFirstPathSep(n))
        return getDrive<StringType>(curDir) + n;
    #endif

    return n;
}

//-----------------------------------------------------------------------------
template<typename StringType> inline
StringType replaceExtention( const StringType &fileName, const StringType &newExt)
{
    return appendExt(getPathFile(fileName), newExt);
}

//-----------------------------------------------------------------------------
template<typename StringType> inline
StringType replaceExt( const StringType &fileName, const StringType &newExt)
{
    return replaceExtention(fileName, newExt);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Делает путь плоским - заменяет разделители пути и спец-имена каталогов на подчеркивания, символы расширения (точка), кроме последего также заменяются
template<typename StringType> inline
StringType flattenPath( StringType       fileName
                      , bool             keepLastExtention = false
                      , const StringType &currentDirAlias  = umba::filename::getNativeCurrentDirAlias<StringType>()
                      , const StringType &parentDirAlias   = umba::filename::getNativeParentDirAlias<StringType>()
                      )
{
    constexpr const auto flattenChar = (typename StringType::value_type)'_';
    fileName = normalizePathSeparators(fileName, (typename StringType::value_type)'/');

    StringType pathFile;
    StringType ext;
    if (keepLastExtention)
    {
        pathFile = getPathFile(fileName);
        ext = getExt(fileName);
    }
    else
    {
        pathFile = fileName;
    }

    std::vector< StringType > parts = umba::string_plus::split(pathFile, (typename StringType::value_type)'/', true /* skipEmpty */ );

    //for(std::size_t i=parts.size(); i-->0; )
    for(auto &p : parts)
    {
        if (p==currentDirAlias || p==parentDirAlias)
        {
            p = StringType(p.size(), flattenChar);
        }

        for(auto &ch : p)
        {
            if (isExtSep(ch))
                ch = flattenChar;
        }
    }

    //isExtSep
    
    // for(auto &p : parts)
    // {
    //     if (p==currentDirAlias || p==parentDirAlias)
    //     {
    //         p = StringType(p.size(), flattenChar);
    //     }
    // }

    if (ext.empty())
        return umba::string_plus::merge(parts,flattenChar);

    return appendExt(umba::string_plus::merge(parts,flattenChar), ext);
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------


#if 0
inline
std::string getPathName( const std::string &s )
{
    if (s.empty())
        return s;

    std::string::size_type pos = s.size();
    for( ; pos; --pos)
    {
        char ch = s[pos-1];
        if (ch=='.')
        {
            return std::string( s, 0, pos-1 ); // cut ext
        }
        if (isPathSep(ch)) // (ch=='\\' || ch=='/') // found path sep before ext sep
        {
            return s;
        }
    }

    return s;
}

//-----------------------------------------------------------------------------
inline
std::string getNameFromFull( const std::string &s )
{
    if (s.empty())
        return s;

    std::string::size_type pos = s.size();
    for( ; pos; --pos)
    {
        char ch = s[pos-1];
        if (isPathSep(ch)) // (ch=='\\' || ch=='/')
        {
            return std::string( s, pos );
        }
    }

    return s;
}

//-----------------------------------------------------------------------------
inline
std::string getFileName( const std::string &name )
{
    std::string f, s;
    splitToPair( getNameFromFull(name), f, s, '.' );
    return f;
}

//-----------------------------------------------------------------------------
inline
std::string getFileExtention( const std::string &name )
{
    std::string f, s;
    splitToPair( getNameFromFull(name), f, s, '.' );
    return s;
}

inline
std::string appendExtention( std::string name, std::string ext )
{
    if (!ext.empty() && ext.front()=='.')
        ext.erase(0,1);
    if (name.empty() || name.back()!='.')
        name.append(".");
    return name+ext;
}

//-----------------------------------------------------------------------------
inline
std::string generateOutputFilename( const std::string &inputFilename, const std::string &outputFilename, const std::string &defName, const std::string &defExt)
{
    std::string outputFinalName = outputFilename;
    std::string ext  = getFileExtention( outputFinalName );
    std::string name = getFileName( outputFinalName );
    std::string path = getPath( outputFinalName );

    std::string inputPath = getPath(inputFilename);


    if (outputFinalName.empty())
        path = getPath(inputFilename);

    if (name.empty())
    {
        outputFinalName = defName;
        if (!path.empty())
            outputFinalName = path + std::string(1, getPathSep()) /* std::string("\\") */  + outputFinalName;
    }

    //if (ext.empty())
    //    ext = defExt;

    if (ext.empty())
    {
        if (!defExt.empty())
            outputFinalName.append(".");
        outputFinalName.append(defExt);
    }

    return outputFinalName;

}

//-----------------------------------------------------------------------------
inline
bool setFileReadOnlyAttr( const std::string &file, bool bSet = true )
{
    #if defined(WIN32) || defined(_WIN32)
    DWORD attrs = GetFileAttributesA( file.c_str() );
    if (attrs==INVALID_FILE_ATTRIBUTES)
       {
        return false; // GetLastError();
       }

    if (bSet)
       attrs |= FILE_ATTRIBUTE_READONLY;
    else
       attrs &= ~FILE_ATTRIBUTE_READONLY;

    BOOL bRes = SetFileAttributesA( file.c_str(), attrs );
    if (!bRes)
       {
        return false; //GetLastError();
       }
    return true;
    #else
    return false; //UNDONE: not implemented
    #endif
}
#endif

//-----------------------------------------------------------------------------
/*
inline
RCODE setFileReadOnlyAttr( const std::wstring &file, bool bSet = true )
{
    DWORD attrs = GetFileAttributesW( file.c_str() );
    if (attrs==INVALID_FILE_ATTRIBUTES)
       {
        return WIN2RC(GetLastError());
       }

    if (bSet)
       attrs |= FILE_ATTRIBUTE_READONLY;
    else
       attrs &= ~FILE_ATTRIBUTE_READONLY;

    BOOL bRes = SetFileAttributesW( file.c_str(), attrs );
    if (!bRes)
       {
        return WIN2RC(GetLastError());
       }
    return EC_OK;
}
*/





} // namespace filename
} // namespace umba
