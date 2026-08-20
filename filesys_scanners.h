/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Cканеры по файловой системе.

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once


#include "filename.h"
#include "filesys.h"
#include "info_log.h"
#include "regex_helpers.h"
#include "umba.h"
#include "string_plus.h"

#include <exception>
//#include <filesystem>
#include <iostream>
#include <list>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>


// umba::filesys::scanners::
namespace umba {
namespace filesys {
namespace scanners {

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! Сканирует каталоги начиная с заданного и выше, в поисках конфиг файла flagsFileName
template<typename StringType> inline
StringType scanForOptionsFile( const StringType &flagsFileName, StringType basePath)
{
    while(!basePath.empty())
    {
        StringType testName = umba::filename::appendPath(basePath,flagsFileName);
        if (umba::filesys::isFileReadable(testName))
        {
            return testName;
        }

        std::string nextBasePath = umba::filename::getPath(basePath);
        if (basePath==nextBasePath)
            nextBasePath.clear();

        basePath = nextBasePath;
    }

    return StringType();
}

//----------------------------------------------------------------------------
//! Сканирует каталоги начиная с заданного и выше, в поисках конфиг файлов flagsFileNames
template<typename StringType> inline
StringType scanForOptionsFile( const std::vector<StringType> &flagsFileNames, StringType basePath)
{
    while(!basePath.empty())
    {
        for(const auto &fileName : flagsFileNames)
        {
            StringType testName = umba::filename::appendPath(basePath,fileName);
            if (umba::filesys::isFileReadable(testName))
            {
                return testName;
            }
        }

        std::string nextBasePath = umba::filename::getPath(basePath);
        if (basePath==nextBasePath)
            nextBasePath.clear();

        basePath = nextBasePath;
    }

    return StringType();
}

//----------------------------------------------------------------------------
template<typename StringType> inline
StringType scanForFlagsFile( const StringType &flagsFileName, StringType basePath)
{
    return scanForOptionsFile(flagsFileName, basePath);
}

//----------------------------------------------------------------------------
template<typename StringType> inline
StringType scanForFlagsFile( const std::vector<StringType> &flagsFileNames, StringType basePath)
{
    return scanForOptionsFile(flagsFileNames, basePath);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template<typename StringType> inline
std::vector< std::basic_regex<typename StringType::value_type> >
fromSimpleMaskToRegexVector(const std::vector<StringType> &regexStrings, bool useAnchoring = false, bool allowRawRegexes = true)
{
    std::vector< std::basic_regex<typename StringType::value_type> > resVec; resVec.reserve(regexStrings.size());

    for(const auto &s : regexStrings)
    {
        resVec.emplace_back(std::basic_regex<typename StringType::value_type>(umba::regex_helpers::expandSimpleMaskToEcmaRegex(s, useAnchoring, allowRawRegexes)));
    }

    return resVec;
}

//----------------------------------------------------------------------------
template<typename StringType> inline
std::vector< std::basic_regex<typename StringType::value_type> >
fromSimpleMaskToRegexMap(const std::vector<StringType> &regexStrings, bool useAnchoring = false, bool allowRawRegexes = true)
{
    std::map< StringType, std::basic_regex<typename StringType::value_type> > resMap;

    for(const auto &s : regexStrings)
    {
        resMap[s] = std::basic_regex<typename StringType::value_type>(umba::regex_helpers::expandSimpleMaskToEcmaRegex(s, useAnchoring, allowRawRegexes));
    }

    return resMap;
}

//auto normalizedEntryName = umba::filename::normalizePathSeparators(entryName,'/');

//----------------------------------------------------------------------------
//! Сканирует каталоги paths в поисках файлов, заданных масками инклюд и эксклюд
/*! Если инклюд маски пусты, этап пропускается. Эксклюд маски обрабатываются в любом случае
*/
template<typename StringType, typename LogMsgType> inline
void scanFolders( const std::vector<StringType> &rootScanPaths
                , const std::vector<StringType> &includeFilesMaskList
                , const std::vector<StringType> &excludeFilesMaskList
                , LogMsgType                    &logMsg           // logMsg or logNul
                , std::vector<StringType>       &foundFiles
                , std::vector<StringType>       &excludedFiles
                , std::set<StringType>          &foundExtentions
                , std::vector<StringType>       *pFoundFilesRootFolders = 0
                , const std::vector<StringType> &excludeFoldersExact = std::vector<StringType>()
                , bool                          scanRecurse          = true
                , bool                          logFoundHeader       = true
                , bool                          addFolders           = true
                , bool                          compareOnlyFilenames = false // not full paths
                )
{
    using namespace umba::omanip;

    //using PathListOrgType = decltype(appConfig.scanPaths);
    //using StringType      = typename PathListOrgType::value_type;

    //std::list<StringType> rootScanPaths( scanPaths.begin(), scanPaths.end() );

    std::unordered_set<StringType>  excludeFoldersExactSet;
    if (scanRecurse)
    {
        std::transform(excludeFoldersExact.begin(), excludeFoldersExact.end(), std::inserter(excludeFoldersExactSet, excludeFoldersExactSet.end()), [](const StringType &str) { return umba::string_plus::tolower_copy(str); });
    }


    std::unordered_map<StringType,std::regex>  excludeRegexes;
    std::unordered_map<StringType,StringType>  excludeOriginalMasks;

    for(auto excludeFileMask : excludeFilesMaskList)
    {
        //auto normalizedName = StringType normalizePathSeparators( const StringType &fileName, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
        //auto regexStr = expandSimpleMaskToEcmaRegex(excludeFileMask);
        auto regexStr = umba::regex_helpers::expandSimpleMaskToEcmaRegex( excludeFileMask, true /* useAnchoring */, true /* allowRawRegexes */ );
        excludeRegexes      [regexStr] = std::regex(regexStr);
        excludeOriginalMasks[regexStr] = excludeFileMask;
    }

    std::unordered_map<StringType,std::regex>  includeRegexes;
    std::unordered_map<StringType,StringType> includeOriginalMasks;

    for(auto includeFileMask : includeFilesMaskList)
    {
        //auto normalizedName = StringType normalizePathSeparators( const StringType &fileName, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
        //auto regexStr = expandSimpleMaskToEcmaRegex(includeFileMask);
        auto regexStr = umba::regex_helpers::expandSimpleMaskToEcmaRegex(includeFileMask, true /* useAnchoring */, true /* allowRawRegexes */ );

        includeRegexes      [regexStr] = std::regex(regexStr);
        includeOriginalMasks[regexStr] = includeFileMask;
    }


    bool bFound = false;

    static const StringType currentDirName = umba::string::make_string<StringType>(".");
    static const StringType upperDirName   = umba::string::make_string<StringType>("..");

    //for( std::list<std::string>::const_iterator rootPathIt=rootScanPaths.begin(); rootPathIt!=rootScanPaths.end(); ++rootPathIt )
    for( typename std::vector<StringType>::const_iterator rootPathIt=rootScanPaths.begin(); rootPathIt!=rootScanPaths.end(); ++rootPathIt )
    {
        std::list<StringType> scanPaths;
        scanPaths.emplace_back(*rootPathIt);

        // template<typename StringType> inline StringType make_string( const std::wstring &str )

        for( typename std::list<StringType>::const_iterator it=scanPaths.begin(); it!=scanPaths.end(); ++it )
        {
            const auto &scanPath = *it; // path

            if (!umba::filesys::enumerateDirectory( scanPath
                                                  , [&](StringType entryName, const umba::filesys::FileStat &fileStat)
                                                    {
                                                        bool bUpperDir   = entryName==upperDirName;
                                                        bool bCurrentDir = entryName==currentDirName;

                                                        auto entryNameOnly     = entryName;
                                                        auto entryNameForMatch = compareOnlyFilenames ? entryName : umba::filename::appendPath(scanPath, entryName);
                                                        entryNameForMatch      = umba::filename::normalizePathSeparators(entryNameForMatch,'/');
                                                        
                                                        entryName = umba::filename::appendPath(scanPath, entryName);

                                                        if (fileStat.fileType==umba::filesys:: /* FileType:: */ FileType::FileTypeDir)
                                                        {
                                                            if (!bUpperDir && !bCurrentDir)
                                                            {
                                                                if (scanRecurse)
                                                                {
                                                                    if (excludeFoldersExactSet.find(umba::string_plus::tolower_copy(entryNameOnly))==excludeFoldersExactSet.end())
                                                                    {
                                                                        scanPaths.push_back(entryName);
                                                                    }
                                                                    else
                                                                    {
                                                                        logMsg << entryName << " - ";
                                                                        logMsg << good_but_warning /* warning */  << "skipped" << notice << /* normal << */  " due to exclude exact folder rule" << normal << "\n";
                                                                    }
                                                                }
                                                            }
                                                            // std::cout << entry.path() << "\n";

                                                            if (!addFolders)
                                                                return true; // continue
                                                        }

                                                        if ( fileStat.fileType!=umba::filesys::FileType::FileTypeFile
                                                          && fileStat.fileType!=umba::filesys::FileType::FileTypeDir
                                                           )
                                                        {
                                                            return true; // continue
                                                        }

                                                        entryName = umba::filename::makeCanonical(entryName);

                                                        if (!bFound)
                                                        {
                                                            bFound = true;

                                                            //if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                            {
                                                                if (logFoundHeader)
                                                                {
                                                                    umba::info_log::printSectionHeader(logMsg, "Found Files");
                                                                    // logMsg << "---------------------\nFound Files:" << endl << "------------" << endl;
                                                                }
                                                            }
                                                        }


                                                        // if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                        {
                                                            logMsg << entryName << " - ";
                                                        }

                                                        // entryNameForMatch
                                                        //auto normalizedEntryName = umba::filename::normalizePathSeparators(entryName,'/');

                                                        //TODO: !!! Нужно что-то решать с отсутствующим расширением

                                                        bool addThisFile = false;
                                                        bool excludedByIncludeMask = false;

                                                        StringType includeRegexStr;
                                                        StringType excludeRegexStr;

                                                        bool matchInclude = true;
                                                        if (!includeRegexes.empty()) // матчим только если не пусто
                                                        {
                                                            matchInclude = umba::regex_helpers::regexMatch(entryNameForMatch,includeRegexes,&includeRegexStr);
                                                        }

                                                        if (!matchInclude)
                                                        {
                                                            // Не подходит под инклюзивную маску
                                                            addThisFile = false;
                                                            excludedByIncludeMask = true;
                                                        }
                                                        else
                                                        {
                                                            addThisFile = true; // Вроде подошло, надо проверить исключения

                                                            if (umba::regex_helpers::regexMatch(entryNameForMatch,excludeRegexes,&excludeRegexStr))
                                                            {
                                                                addThisFile = false;
                                                                excludedByIncludeMask = false;
                                                            }
                                                        }

                                                        if (addThisFile)
                                                        {
                                                            foundFiles.emplace_back(entryName);
                                                            if (pFoundFilesRootFolders)
                                                            {
                                                                pFoundFilesRootFolders->emplace_back(*rootPathIt);
                                                            }

                                                            auto ext = umba::filename::getExt(entryName);

                                                            foundExtentions.insert(ext);

                                                            //if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                            {
                                                                if (ext.empty())
                                                                    ext = "<EMPTY>";
                                                                else
                                                                    ext = umba::string_plus::make_string<StringType>(".") + ext;

                                                                logMsg << good << "added" << normal;
                                                                logMsg << " (" << notice << ext << normal << ")";
                                                                if (!includeRegexStr.empty())
                                                                {
                                                                    // orgMask = includeOriginalMasks[includeRegexStr]
                                                                    logMsg << notice << " due to include mask '" << includeOriginalMasks[includeRegexStr] << "' (" << includeRegexStr << ")" << normal;
                                                                }

                                                                if (ext.empty())
                                                                {
                                                                    logMsg << " - !note: empty extention: " << notice << entryName << normal << "";
                                                                    //logMsg << "\n";
                                                                }

                                                                logMsg << "\n";

                                                            }
                                                        }
                                                        else
                                                        {
                                                            excludedFiles.push_back(entryName);

                                                            //if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                            {
                                                                if (excludedByIncludeMask)
                                                                {
                                                                    logMsg << good_but_notice /* warning */  << "skipped" << notice << /* normal << */  " due to include masks" << normal << "\n";
                                                                }
                                                                else
                                                                {
                                                                    logMsg << good_but_warning /* warning */  << "skipped" << notice << /* normal << */  " due to exclude mask '" << excludeOriginalMasks[excludeRegexStr] << "' (" << excludeRegexStr << ")" << normal << "\n";
                                                                }
                                                            }
                                                        }

                                                        return true;
                                                    }
                                                  )
               )
            {
            }

        } // for

    } // for


}

//----------------------------------------------------------------------------
//! Сканирует каталоги в поисках файлов, заданных масками инклюд и эксклюд - appConfig.includeFilesMaskList и appConfig.excludeFilesMaskList
/*! Если инклюд маски пусты, этап пропускается. Эксклюд маски обрабатываются в любом случае
*/
template<typename StringType, typename AppConfigType, typename LogMsgType> inline
void scanFolders( const AppConfigType            &appConfig        // with includeFilesMaskList and excludeFilesMaskList
                , LogMsgType                     &logMsg           // logMsg or logNul
                , std::vector<StringType>        &foundFiles
                , std::vector<StringType>        &excludedFiles
                , std::set<StringType>           &foundExtentions
                , std::vector<StringType>        *pFoundFilesRootFolders = 0
                , const std::vector<StringType>  &excludeFoldersExact = std::vector<StringType>()
                , bool                           scanRecurse          = true
                , bool                           logFoundHeader       = true
                , bool                           addFolders           = true
                , bool                           compareOnlyFilenames = false // not full paths
                )
{
    scanFolders( appConfig.scanPaths
               , appConfig.includeFilesMaskList
               , appConfig.excludeFilesMaskList
               , logMsg           // logMsg or logNul
               , foundFiles
               , excludedFiles
               , foundExtentions
               , pFoundFilesRootFolders
               , excludeFoldersExact
               , scanRecurse
               , logFoundHeader
               , addFolders
               , compareOnlyFilenames
               );

    #if 0
    using namespace umba::omanip;

    //using PathListType = decltype()

    using PathListOrgType = decltype(appConfig.scanPaths);
    using StringType      = typename PathListOrgType::value_type;

    std::list<StringType> rootScanPaths( appConfig.scanPaths.begin(), appConfig.scanPaths.end() );

    std::map<StringType,std::regex>  excludeRegexes;
    std::map<StringType,StringType> excludeOriginalMasks;

    for(auto excludeFileMask : appConfig.excludeFilesMaskList)
    {
        //auto normalizedName = StringType normalizePathSeparators( const StringType &fileName, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
        //auto regexStr = expandSimpleMaskToEcmaRegex(excludeFileMask);
        auto regexStr = umba::regex_helpers::expandSimpleMaskToEcmaRegex( excludeFileMask, true /* useAnchoring */, true /* allowRawRegexes */ );
        excludeRegexes      [regexStr] = std::regex(regexStr);
        excludeOriginalMasks[regexStr] = excludeFileMask;
    }

    std::map<StringType,std::regex>  includeRegexes;
    std::map<StringType,StringType> includeOriginalMasks;

    for(auto includeFileMask : appConfig.includeFilesMaskList)
    {
        //auto normalizedName = StringType normalizePathSeparators( const StringType &fileName, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )
        //auto regexStr = expandSimpleMaskToEcmaRegex(includeFileMask);
        auto regexStr = umba::regex_helpers::expandSimpleMaskToEcmaRegex(includeFileMask, true /* useAnchoring */, true /* allowRawRegexes */ );

        includeRegexes      [regexStr] = std::regex(regexStr);
        includeOriginalMasks[regexStr] = includeFileMask;
    }


    bool bFound = false;



    for( std::list<std::string>::const_iterator rootPathIt=rootScanPaths.begin(); rootPathIt!=rootScanPaths.end(); ++rootPathIt )
    {
        std::list<StringType> scanPaths;
        scanPaths.emplace_back(*rootPathIt);

        for( std::list<std::string>::const_iterator it=scanPaths.begin(); it!=scanPaths.end(); ++it )
        {
            const auto &scanPath = *it; // path

            if (!umba::filesys::enumerateDirectory( scanPath
                                                  , [&](std::string entryName, const umba::filesys::FileStat &fileStat)
                                                    {
                                                        entryName = umba::filename::appendPath(scanPath, entryName);

                                                        if (fileStat.fileType==umba::filesys:: /* FileType:: */ FileTypeDir)
                                                        {
                                                            scanPaths.push_back(entryName);
                                                            // std::cout << entry.path() << "\n";
                                                            return true; // continue
                                                        }

                                                        if (fileStat.fileType!=umba::filesys:: /* FileType:: */ FileTypeFile)
                                                        {
                                                            return true; // continue
                                                        }

                                                        entryName = umba::filename::makeCanonical(entryName);

                                                        if (!bFound)
                                                        {
                                                            bFound = true;

                                                            //if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                            {
                                                                umba::info_log::printSectionHeader(logMsg, "Found Files");
                                                                // logMsg << "---------------------\nFound Files:" << endl << "------------" << endl;
                                                            }
                                                        }


                                                        // if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                        {
                                                            logMsg << entryName << " - ";
                                                        }

                                                        auto normalizedEntryName = umba::filename::normalizePathSeparators(entryName,'/');

                                                        //TODO: !!! Нужно что-то решать с отсутствующим расширением

                                                        bool addThisFile = false;
                                                        bool excludedByIncludeMask = false;

                                                        std::string includeRegexStr;
                                                        std::string excludeRegexStr;

                                                        bool matchInclude = true;
                                                        if (!includeRegexes.empty()) // матчим только если не пусто
                                                        {
                                                            matchInclude = umba::regex_helpers::regexMatch(normalizedEntryName,includeRegexes,&includeRegexStr);
                                                        }

                                                        if (!matchInclude)
                                                        {
                                                            // Не подходит под инклюзивную маску
                                                            addThisFile = false;
                                                            excludedByIncludeMask = true;
                                                        }
                                                        else
                                                        {
                                                            addThisFile = true; // Вроде подошло, надо проверить исключения

                                                            if (umba::regex_helpers::regexMatch(normalizedEntryName,excludeRegexes,&excludeRegexStr))
                                                            {
                                                                addThisFile = false;
                                                                excludedByIncludeMask = false;
                                                            }
                                                        }

                                                        if (addThisFile)
                                                        {
                                                            foundFiles.emplace_back(entryName);
                                                            if (pFoundFilesRootFolders)
                                                            {
                                                                pFoundFilesRootFolders->emplace_back(*rootPathIt);
                                                            }

                                                            auto ext = umba::filename::getExt(entryName);

                                                            foundExtentions.insert(ext);

                                                            //if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                            {
                                                                if (ext.empty())
                                                                    ext = "<EMPTY>";
                                                                else
                                                                    ext = std::string(".") + ext;

                                                                logMsg << good << "added" << normal;
                                                                logMsg << " (" << notice << ext << normal << ")";
                                                                if (!includeRegexStr.empty())
                                                                {
                                                                    // orgMask = includeOriginalMasks[includeRegexStr]
                                                                    logMsg << " due to include mask '" << includeOriginalMasks[includeRegexStr] << "' (" << includeRegexStr << ")" << normal;
                                                                }

                                                                if (ext.empty())
                                                                {
                                                                    logMsg << " - !note: empty extention: " << notice << entryName << normal << "";
                                                                    //logMsg << "\n";
                                                                }

                                                                logMsg << "\n";

                                                            }
                                                        }
                                                        else
                                                        {
                                                            excludedFiles.push_back(entryName);

                                                            //if (appConfig.testVerbosity(VerbosityLevel::detailed))
                                                            {
                                                                if (excludedByIncludeMask)
                                                                {
                                                                    logMsg << notice << "skipped" <<  /* normal << */  " due to include masks" << normal << "\n";
                                                                }
                                                                else
                                                                {
                                                                    logMsg << notice << "skipped" <<  /* normal << */  " due to exclude mask '" << excludeOriginalMasks[excludeRegexStr] << "' (" << excludeRegexStr << ")" << normal << "\n";
                                                                }
                                                            }
                                                        }

                                                        return true;
                                                    }
                                                  )
               )
            {
            }

        } // for

    } // for

    #endif
}




} // namespace scanners
} // namespace filesys
} // namespace umba
