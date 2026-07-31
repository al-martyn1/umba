/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Поиск месторасположения self EXE-шника

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

#include "filename.h"
#include "filesys.h"
#include "env.h"
#include "macros.h"
#include "string_plus.h"
#include "zz_inc_windows.h"
#include "debug_helpers.h"
//
//#include <filesystem>
#include <map>





#if defined(WIN32) || defined(_WIN32)

    #define UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME      "conf"

    #define UMBA_PROGRAM_LOCATION_INIT_IMPL( argv0 )        do {} while(0)
    #define UMBA_PROGRAM_LOCATION_INIT( argc, argv )        do {} while(0)

#else

    #if defined(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME_FORCE_CONF)
        #define UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME      "conf"
    #else
        #define UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME      "etc"
    #endif

    #define UMBA_PROGRAM_LOCATION_INIT_IMPL( argv0 )        umba::program_location::setArgv0(argv[0])
    #define UMBA_PROGRAM_LOCATION_INIT( argc, argv )             \
                if (argc>0)                                      \
                {                                                \
                    UMBA_PROGRAM_LOCATION_INIT_IMPL(argv[0]);    \
                }

#endif

#define UMBA_PROGRAM_LOCATION_DEF_LOG_FOLDER_NAME       "log"
#define UMBA_PROGRAM_LOCATION_DEF_CACHE_FOLDER_NAME     "cache"


// umba::program_location::
namespace umba
{
namespace program_location
{


template<typename StringType>
StringType& getArgv0_nc();

template<> inline
std::string& getArgv0_nc<std::string>()
{
    static std::string argv0;
    return argv0;
}

template<> inline
std::wstring& getArgv0_nc<std::wstring>()
{
    static std::wstring argv0;
    return argv0;
}

template<typename StringType>
const StringType& getArgv0()
{
    return getArgv0_nc<StringType>();
}


template<typename StringType> inline
void setArgv0(const StringType &argv0)
{
    // Try to keep both versions the same

    if (getArgv0_nc<std::string>().empty()) // prevent to overwrite allready initialized argv0
    {
        getArgv0_nc<std::string>() = umba::string_plus::make_string<std::string>(argv0);
    }

    if (getArgv0_nc<std::wstring>().empty()) // prevent to overwrite allready initialized argv0
    {
        getArgv0_nc<std::wstring>() = umba::string_plus::make_string<std::wstring>(argv0);
    }
}



// #include "fsUtils.h"

template<typename StringType>
StringType getExeName(); // { return throw std::runtime_error("getExeName: not specified"); StringType(); }


#if defined(WIN32) || defined(_WIN32)


    template<> inline
    std::string getExeName<std::string>()
    {
        char buf[4096];
        const std::size_t bufSize = sizeof(buf)/sizeof(buf[0]);

        DWORD res = GetModuleFileNameA( 0, buf, bufSize );
        if (res==bufSize)
            buf[bufSize-1] = 0;

        return std::string(buf);
    }

    template<> inline
    std::wstring getExeName<std::wstring>()
    {
        wchar_t buf[4096];
        const std::size_t bufSize = sizeof(buf)/sizeof(buf[0]);

        DWORD res = GetModuleFileNameW( 0, buf, bufSize );
        if (res==bufSize)
            buf[bufSize-1] = 0;

        return std::wstring(buf);
    }


#else // Generic POSIX - Linups etc


    template<> inline
    std::string getExeName<std::string>()
    {
        // !!! Надо сделать через dl_*
        return getArgv0<std::string>();
    }

#endif

template<typename StringType> inline
void detectLocation( StringType &exeFullName, StringType &progBinPath, StringType &progRootPath )
{
    exeFullName = getExeName<StringType>();
    if (!exeFullName.empty())
    {
        progBinPath = umba::filename::getPath( exeFullName );
        bool isExeFolderBin = false; // По умолчанию бинарник лежит сразу в главном фолдере дистра

        if (!progBinPath.empty())
        {
            // При отладке, когда бинарники не разложены как в дистре, а лежат по каталогам конфигураций сборки в общем надкаталоге,
            // удобно иметь иметь conf на уровень выше, чтобы он был общий для всех сборок

            auto cmpHelper = [](const char *strForCmp)
            {
                return umba::string_plus::tolower_copy( umba::string_plus::make_string<StringType>(strForCmp) );
            };

            StringType exeFolderNameOnly = umba::string_plus::tolower_copy /* toLower */ (umba::filename::getFileName( progBinPath ));

            if ( exeFolderNameOnly==cmpHelper("bin")

              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("Debug"))
              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("Release"))
              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("Unicode"))
              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("Ansi"))

              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("Debug"))
              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("Release"))
              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("Unicode"))
              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("Ansi"))

              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("RelWithDebInfo"))
              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("RelWithDebInfo"))

              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("RelWithDbgInfo"))
              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("RelWithDbgInfo"))

              || umba::string_plus::starts_with(exeFolderNameOnly, cmpHelper("MinSizeRel"))
              || umba::string_plus::ends_with  (exeFolderNameOnly, cmpHelper("MinSizeRel"))

               )
            {
                //
                isExeFolderBin = true;
            }
        }

        if (isExeFolderBin)
        {
            progRootPath    = umba::filename::getPath( progBinPath );
        }
        else
        {
            progRootPath    = progBinPath;
        }

        //progConfPath    = progRootPath + std::string("\\conf");
        //progIncludePath = progRootPath + std::string("\\include");
    }
}



struct BuiltinOptionsLocationFlag
{
    const static unsigned     appGlobal     = 0x0001;
    const static unsigned     customGlobal  = 0x0002;
    const static unsigned     userLocal     = 0x0004;
    const static unsigned     all           = 0x0007;
};



template<typename StringType>
struct ProgramLocation
{

    StringType  exeFullName; //!< Executable full name with path
    StringType  exeFileName; //!< Executable file name (with extention, eg 'exe')
    StringType  exeName    ; //!< Executable file name only (without extention)

    StringType  rootPath   ; //!< App root path, eg C:\Program Files\AppName or \usr\opt\AppName
    StringType  binPath    ; //!< App bin path, eg C:\Program Files\AppName\bin (or may be the same as rootPath) or \usr\opt\AppName\bin
    StringType  confPath   ; //!< App conf path, depending on custom settings, eg  C:\Program Files\AppName\conf or \usr\opt\AppName\conf
    StringType  userConf   ; //!< User config file/folder, eg C:\Users\UserName\.exeName
    StringType  logPath    ; //!< App log path, depending on custom settings, eg  C:\Program Files\AppName\log or \usr\opt\AppName\log
    StringType  cachePath  ; //!< App log path, depending on custom settings, eg  C:\Program Files\AppName\cache or \usr\opt\AppName\cache

    StringType  cwd        ; //!< Current working dir

    bool        useUserFolder = false; //!< If false single file(s) used


    umba::macros::StringStringMap<StringType> getProgramLocationMacros() const
    {
        // std::map<StringType, StringType> m;
        umba::macros::StringStringMap<StringType> m;

        m[umba::string_plus::make_string<StringType>("AppName")] = exeName;


        m[umba::string_plus::make_string<StringType>("AppPath")] = rootPath; // For compatibility

        m[umba::string_plus::make_string<StringType>("AppBinPath"   )] = binPath ;
        m[umba::string_plus::make_string<StringType>("AppRootPath"  )] = rootPath;
        m[umba::string_plus::make_string<StringType>("AppConfPath"  )] = confPath;

        m[umba::string_plus::make_string<StringType>("UserHomePath" )] = umba::filesys::nameConvert<StringType>(umba::filesys::getCurrentUserHomeDirectory());

        // stripLastPathSep
        m[umba::string_plus::make_string<StringType>("AppBin"       )] = m[umba::string_plus::make_string<StringType>("AppBinPath"   )];
        m[umba::string_plus::make_string<StringType>("AppRoot"      )] = m[umba::string_plus::make_string<StringType>("AppRootPath"  )];
        m[umba::string_plus::make_string<StringType>("AppConf"      )] = m[umba::string_plus::make_string<StringType>("AppConfPath"  )];

        m[umba::string_plus::make_string<StringType>("AppUserHome"     )] = m[umba::string_plus::make_string<StringType>("UserHomePath" )];


        m[umba::string_plus::make_string<StringType>("AppTempPath"     )] = umba::filesys::nameConvert<StringType>(umba::filesys::getTempFolderPath());
        m[umba::string_plus::make_string<StringType>("AppTemp"         )] = m[umba::string_plus::make_string<StringType>("TempPath"     )];


        StringType logPath;

        #if defined(WIN32) || defined(_WIN32)

            StringType logRoot;

            if (!umba::env::getVar(umba::string_plus::make_string<StringType>("LOCALAPPDATA"), logRoot))
            {
                //return res;
                logRoot = umba::string_plus::make_string<StringType>("C:\\Log");
            }

            logPath = umba::filename::appendPath(umba::filename::appendPath(logRoot, exeName), umba::string_plus::make_string<StringType>("log") );

        #else

            logPath = umba::filename::appendPath(umba::filename::appendPath(umba::string_plus::make_string<StringType>("/var/log"), exeName) );

        #endif

        m[umba::string_plus::make_string<StringType>("AppLogPath"      )] = logPath;

        // PersistLogPath
        // LogPath

        // UserTemp

        // Windows
        //   System-wide persistent
        //   System-wide
        //   User

        // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppatha

        // Про логи - http://rsdn.org/forum/setup/8192177.1
        // %LOCALAPPDATA%

        // Temp
        // https://superuser.com/questions/332610/where-is-the-temporary-directory-in-linux/332616

        // Temp
        // In Unix and Linux, the global temporary directories are /tmp and /var/tmp.
        // Web browsers periodically write data to the tmp directory during page views and downloads.
        // Typically, /var/tmp is for persistent files (as it may be preserved over reboots), and /tmp is for more temporary files.

        // In addition, a user can set their TMPDIR environment variable to point to a preferred directory
        // (where the creation and modification of files is allowed).

        // https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard

        // /var/log - Log files. Various logs.

        // TMP =C:\Users\martinov\AppData\Local\Temp
        // TEMP=C:\Users\martinov\AppData\Local\Temp

        // https://en.wikipedia.org/wiki/Temporary_folder

        //umba::filesys::getCurrentUserHomeDirectory();

        // StringType res;
        // if (umba::env::getVar(make_string<StringType>("HOME"), res))
        // {
        //     return res;
        // }


        return m;
    }

    umba::macros::StringStringMap<StringType> mergeProgramLocationMacros( umba::macros::StringStringMap<StringType> mergeTo ) const
    {
         auto locationMacros = getProgramLocationMacros();

         for( auto [name, val] : locationMacros)
             mergeTo[name] = val;

         return mergeTo;
    }


    //! returns AppRoot folder
    StringType getAppRoot() const
    {
        return rootPath;
    }

    StringType getAppConfPath() const
    {
        return confPath;
    }

    //! Creates full path under AppRoot folder
    StringType  getAppRootSubName(const StringType &subName) const
    {
        return umba::filename::appendPath(rootPath, subName);
    }

    //! Creates full path under AppRoot folder
    StringType  getAppRootSubName(const StringType &subName, const StringType &subExt) const
    {
        return getAppRootSubName(umba::filename::appendExt(subName, subExt));
    }

    //! Creates full path under CWD if taken path is relative
    StringType  makeAbsPath( const StringType &path ) const
    {
        return umba::filename::makeAbsPath(path, cwd);
    }


    //! If userConfFileOnly==true, used $(Home)\.$(AppExeName).options, else used $(Home)\.$(AppExeName)\builtin.options
    std::vector<StringType> getBuiltinOptionsFilenames( unsigned flags, bool _useUserFolder ) const
    {
        using namespace umba::filename   ;
        using namespace umba::string_plus;

        std::vector<StringType> res;

        // Global std options file
        if (flags&BuiltinOptionsLocationFlag::appGlobal)
        {
            #if !defined(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS)
                res.push_back( appendPath(  /* programLocationInfo. */ confPath, appendExt(  /* programLocationInfo. */ exeName, make_string<StringType>("options") ) ) );
            #else
                res.push_back( appendPath(  /* programLocationInfo. */ confPath, appendExt(  /* programLocationInfo. */ umba::filesys::nameConvert<StringType>(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS), make_string<StringType>("options") ) ) );
            #endif
        }

        // Global user options file
        if (flags&BuiltinOptionsLocationFlag::customGlobal)
        {
            #if !defined(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS)
                res.push_back( appendPath(  /* programLocationInfo. */ confPath, appendExt(  /* programLocationInfo. */ exeName, make_string<StringType>("custom.options") ) ) );
            #else
                res.push_back( appendPath(  /* programLocationInfo. */ confPath, appendExt(  /* programLocationInfo. */ umba::filesys::nameConvert<StringType>(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS), make_string<StringType>("custom.options") ) ) );
            #endif
        }
        // Local user options file
        // $(HOME)\.$(AppExeName).options
        // or
        // $(HOME)\.$(AppExeName)\.options
        if (flags&BuiltinOptionsLocationFlag::userLocal)
        {
            if (!_useUserFolder /* userConfFileOnly */ )
            {
                res.push_back( appendExt(  /* programLocationInfo. */ userConf, make_string<StringType>(".options") ) );
            }
            else
            {
                res.push_back( appendPath(  /* programLocationInfo. */ userConf, make_string<StringType>(".options") ) );
            }
        }

        return res;
    }

    std::vector<StringType> getBuiltinOptionsFilenames( unsigned flags ) const
    {
        return getBuiltinOptionsFilenames( flags, useUserFolder );
    }

    //! Can return wrong result if multiple flags are taken
    StringType getBuiltinOptionsFilename( unsigned flag, bool _useUserFolder ) const
    {
        auto names = getBuiltinOptionsFilenames(flag, _useUserFolder);
        if (names.empty())
            return StringType();
         return umba::filename::normalizePathSeparators(names[0],'/');
    }

    StringType getBuiltinOptionsFilename( unsigned flag ) const
    {
        return getBuiltinOptionsFilename( flag, useUserFolder );
    }

    StringType getConfFilename( StringType fileName // name relative to conf root
                              , bool useUserConf
                              ) const
    {
        StringType confRoot = useUserConf ? userConf : confPath;
        return umba::filename::appendPath(/* programLocationInfo. */ confRoot, fileName);
    }



    template<typename StreamType>
    StreamType& print(StreamType &os) const
    {
        os << "Executable Full PathName        : " << exeFullName << "\n";
        os << "Executable Full Name            : " << exeFileName << "\n";
        os << "Executable Name                 : " << exeName     << "\n";

        os << "Application Distr Root Folder   : " << rootPath    << "\n";
        os << "Application Binaries Folder     : " << binPath     << "\n";
        os << "Application Conf Folder         : " << confPath    << "\n";
        os << "Application User File/Folder    : " << userConf    << "\n";
        os << "Application Current Working Dir : " << cwd         << "\n";

        auto builtinOptionFilenames = getBuiltinOptionsFilenames(false);

        for(const auto &fn : builtinOptionFilenames)
            os << "Builtin Options File v1         : " << fn      << "\n";

        builtinOptionFilenames = getBuiltinOptionsFilenames(true);

        for(const auto &fn : builtinOptionFilenames)
            os << "Builtin Options File v2         : " << fn      << "\n";

        return os;
    }

};

template<typename StreamType, typename StringType> inline
StreamType& operator<<(StreamType &os, const ProgramLocation<StringType> &pl)
{
    return pl.print(os);
}


//----------------------------------------------------------------------------
template<typename StringType> inline
const std::vector<StringType>& getConfFolderNames( const StringType &confFolderName )
{
    static std::vector<StringType> confNames;

    if (!confNames.empty())
        return confNames;

    confNames.emplace_back(confFolderName);
    confNames.emplace_back(umba::string_plus::make_string<StringType>("conf"));
    confNames.emplace_back(umba::string_plus::make_string<StringType>("config"));
    confNames.emplace_back(umba::string_plus::make_string<StringType>("cfg"));
    confNames.emplace_back(umba::string_plus::make_string<StringType>("etc"));

    return confNames;
}

//----------------------------------------------------------------------------
template<typename StringType> inline
StringType findAppConfPath( const StringType &appRoot, const StringType &confFolderName )
{
    std::vector<StringType> confCandisDatton;
    std::vector<StringType> confRoots;
    StringType              curConfRoot = appRoot;

    confRoots.emplace_back(curConfRoot);

    // $(ProjectRoot)\.out\msvc2019\x64
    // $(ProjectRoot)\.out\msvc2019
    // $(ProjectRoot)\.out
    // $(ProjectRoot)

    #if !defined(UMBA_PROGRAM_LOCATION_CONF_LOOKUP_LEVELS_MAX)

        #define UMBA_PROGRAM_LOCATION_CONF_LOOKUP_LEVELS_MAX 4u

    #endif

    for( auto i=1u; i<=UMBA_PROGRAM_LOCATION_CONF_LOOKUP_LEVELS_MAX; ++i)
    {
        curConfRoot = umba::filename::appendPath( curConfRoot, umba::string_plus::make_string<StringType>("..") );
        curConfRoot = umba::filename::makeCanonical(curConfRoot);
        confRoots.emplace_back(curConfRoot);
    }

    //for( avito )

    auto confNames = getConfFolderNames(confFolderName);

    for( auto confRootCandy : confRoots )
    {
        for( auto confName : confNames )
        {
            auto candisDatton = umba::filename::appendPath( confRootCandy, confName );
            candisDatton      = umba::filename::makeCanonical(candisDatton);
            //if (std::filesystem::exists(candisDatton))
            if (umba::filesys::isPathDirectory(candisDatton))
                return candisDatton;
        }
    }

    return umba::filename::appendPath( appRoot, confFolderName );

}


//----------------------------------------------------------------------------
//! Возвращает информацию по расположению исполняемого файла и основных файлов/каталогов программы
template<typename StringType> inline
ProgramLocation<StringType> getProgramLocationImpl( const StringType &argv0
                                                  , bool             useUserFolder    = false        //!< If false single file(s) used
                                                  , StringType       overrideExeName  = StringType() //!< Used to make single user conf name for multiple exe's
                                                  , const StringType &confFolderName  = umba::string_plus::make_string<StringType>(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME )
                                                  , const StringType &logFolderName   = umba::string_plus::make_string<StringType>(UMBA_PROGRAM_LOCATION_DEF_LOG_FOLDER_NAME  )
                                                  , const StringType &cacheFolderName = umba::string_plus::make_string<StringType>(UMBA_PROGRAM_LOCATION_DEF_CACHE_FOLDER_NAME)
                                                  )
{
    UMBA_USED(argv0);
    UMBA_PROGRAM_LOCATION_INIT_IMPL( argv0 );

    ProgramLocation<StringType> loc;

    // Detect main location values
    detectLocation( loc.exeFullName, loc.binPath, loc.rootPath );

    loc.exeFileName = umba::filename::getFileName(loc.exeFullName);


    //StringType confRoot = findAppConfPath(loc.rootPath);
    loc.confPath    = findAppConfPath(loc.rootPath, confFolderName); // umba::filename::appendPath(loc.rootPath, confFolderName);

    auto exeName    = umba::filename::getName(loc.exeFullName);
    if (!overrideExeName.empty())
        exeName = overrideExeName;

    loc.exeName     = exeName;


    StringType userHomeDir = umba::filesys::nameConvert<StringType>(umba::filesys::getCurrentUserHomeDirectory());

    #if !defined(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS)
        loc.userConf    = umba::filename::appendPath(userHomeDir, umba::filename::appendExt(StringType(), exeName) );
        loc.logPath     = umba::filename::appendPath(userHomeDir, umba::filename::appendExt(StringType(), exeName) );
        loc.cachePath   = umba::filename::appendPath(userHomeDir, umba::filename::appendExt(StringType(), exeName) );
    #else
        loc.userConf    = umba::filename::appendPath(userHomeDir, umba::filename::appendExt(StringType(), umba::filesys::nameConvert<StringType>(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS)) );
        loc.logPath     = umba::filename::appendPath(userHomeDir, umba::filename::appendExt(StringType(), umba::filesys::nameConvert<StringType>(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS)) );
        loc.cachePath   = umba::filename::appendPath(userHomeDir, umba::filename::appendExt(StringType(), umba::filesys::nameConvert<StringType>(UMBA_PROGRAM_LOCATION_OVERRIDEN_EXECUTABLE_NAME_FOR_CONFIGS)) );
    #endif

    loc.logPath     = umba::filename::appendPath(loc.logPath  , logFolderName  );
    loc.cachePath   = umba::filename::appendPath(loc.cachePath, cacheFolderName);


    loc.cwd         = umba::filesys::nameConvert<StringType>(umba::filesys::getCurrentDirectory());

    loc.useUserFolder = useUserFolder;

    return loc;

}

//! Возвращает информацию по расположению исполняемого файла и основных файлов/каталогов программы
inline
ProgramLocation<std::string> getProgramLocation( int argc, char    **argv
                                               , bool              useUserFolder   = false        //!< If false single file(s) used
                                               , std::string       overrideExeName = std::string()
                                               , const std::string &confFolderName = umba::string_plus::make_string<std::string>(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME)
                                               )
{
    UMBA_USED(argv);

    if (argc>0)
    {
        UMBA_PROGRAM_LOCATION_INIT_IMPL(std::string(argv[0]));
    }

    return getProgramLocationImpl( getArgv0<std::string>(), useUserFolder, overrideExeName, confFolderName );

}

//! Возвращает информацию по расположению исполняемого файла и основных файлов/каталогов программы
inline
ProgramLocation<std::wstring> getProgramLocation( int argc, wchar_t **argv
                                                , bool              useUserFolder    = false        //!< If false single file(s) used
                                                , std::wstring overrideExeName       = std::wstring()
                                                , const std::wstring &confFolderName = umba::string_plus::make_string<std::wstring>(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME)
                                                )
{
    UMBA_USED(argv);

    if (argc>0)
    {
        UMBA_PROGRAM_LOCATION_INIT_IMPL(std::wstring(argv[0]));
    }

    return getProgramLocationImpl( getArgv0<std::wstring>(), useUserFolder, overrideExeName, confFolderName );

}



// void detectLocation( StringType &exeFullName, StringType &progBinPath, StringType &progRootPath )


} // namespace program_location
} // namespace umba

// umba::program_location::


