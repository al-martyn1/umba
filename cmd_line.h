/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Разная шляпа по разбору командной строки для консольных утилит

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

#include "format_utils.h"
#include "parse_utils.h"
#include "program_location.h"
#include "string_plus.h"
#include "text_utils.h"
#include "utf8.h"
#include "filesys.h"
#include "filename.h"
#include "debug_helpers.h"
#include "rule_of_five.h"
#include "shellapi.h"
#include "string.h"

//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <iterator>
#include <functional>
#include <initializer_list>
#include <exception>
#include <stdexcept>

//
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(UMBA_DISABLE_AUTO_ENCODING)
    #include "encoding/encoding.h"
#endif

// #include "trims.h"
// #include "splits.h"
// #include "textUtils.h"
// #include "sys_paths.h"
// #include "cpp.h"



extern std::string appFullName  ;
extern std::string appVersion   ;
extern std::string appCommitHash;  //UNDONE
extern std::string appBuildDate ;
extern std::string appBuildTime ;


/*

 Использование


 1) Создаём наследника umba::command_line::CommandLineOptionCollectorImplBase
    Он просто обрабатывает ошибки задания повторяющихся значений - примерно как
    компилятор ругается на дублирующиеся case'ы в switch

    class CommandLineOptionCollector : public umba::command_line::CommandLineOptionCollectorImplBase
    {
    protected:
       virtual void onOptionDup( const std::string &opt ) override
       {
           LOG_ERR_OPT<<"Duplicated option key - '"<<opt<<"'\n";
           // std::cout << <<"Duplicated option key - '"<<opt<<"'\n";
           throw std::runtime_error("Duplicated option key");
       }
    };


 2) Создаём ArgParser - функциональный объект, обрабатывающий аргумент

    struct ArgParser
    {

        // Для корректной обработки путей - в респонз файлах считается, что пути указаны относительно него
        std::stack<std::string> optFiles;

        std::string makeAbsPath( std::string p )
        {
            std::string basePath;

            if (optFiles.empty())
                basePath = umba::filesys::getCurrentDirectory<std::string>();
            else
                basePath = umba::filename::getPath(optFiles.top());

            return umba::filename::makeAbsPath( p, basePath );
        }


        // 0 - ok, 1 normal stop, -1 - error
        template<typename ArgsParser>
        int operator()( const std::string                               &a           //!< строка - текущий аргумент
                      , umba::command_line::CommandLineOption           &opt         //!< Объект-опция, содержит разобранный аргумент и умеет отвечать на некоторые вопросы
                      , ArgsParser                                      &argsParser  //!< Класс, который нас вызывает, содержит некоторый контекст
                      , umba::command_line::ICommandLineOptionCollector *pCol        //!< Коллектор опций - собирает инфу по всем опциям и готов вывести справку
                      , bool fBuiltin
                      , bool ignoreInfos
                      )
        {
            return -1;
        }
    };


 3) В операторе operator() пишем

    if (opt.isOption())
    {
        // Тут будет обработка опций
    }
    else if (opt.isResponseFile())
    {
        // Если не нужна обработка респонз файлов, просто вернуть -1;

        std::string optFileName = makeAbsPath(opt.name);

        optFiles.push(optFileName);

        auto parseRes = argsParser.parseBuiltinsFile( optFileName );

        optFiles.pop();

        if (!parseRes)
            return -1;

        if (argsParser.mustExit)
            return 1;

        return 0;

    }

    // А тут просто собираем всё, что не опция или респонз файл - простые аргументы

    appConfig.inpu.push_back(makeAbsPath(a));

    return 0;



 4) Обрабатываем опции


        std::string errMsg;
        int intVal;

        if (opt.name.empty())
        {
            LOG_ERR_OPT<<"invalid (empty) option name\n";
            return -1;
        }

       if ( opt.isOption("quet")                   // Длинная опция  - --quet   \  допустимо использовать
         || opt.isOption('q')                      // Короткая опция - -q       /  только одно
         || opt.setDescription("Operate quetly"))  // Описание для автогенерируемой справки
        {
            argsParser.quet = true;
            appConfig.setOptQuet(true);
        }
        else if (opt.isOption("version") || opt.isOption('v') || opt.setDescription("Show version info"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!ignoreInfos)
            {
                printOnlyVersion();
                return 1;
            }
        }
        else if (opt.isOption("where") || opt.setDescription("Show where the executable file is"))
        {
            if (argsParser.hasHelpOption) return 0;

            LOG_MSG_OPT << programLocationInfo.exeFullName << "\n";
            return 0;
        }

        else if (opt.isOption("no-builtin-options")        || opt.setDescription("Don't parse predefined options from main distribution options file 'conf/umba-pretty-headers.options' and 'conf/umba-pretty-headers.user'"))
        { } // simple skip - обработка уже сделана

        else if (opt.isOption("no-custom-builtin-options") || opt.setDescription("Don't parse predefined options from custom global options file 'conf/umba-pretty-headers.custom.options'"))
        { } // simple skip - обработка уже сделана

        else if (opt.isOption("no-user-builtin-options")   || opt.setDescription("Don't parse predefined options from user local options file '$(HOME)\.$(AppExeName).options' or '$(HOME)\.$(AppExeName)\.options'"))
        { } // simple skip - обработка уже сделана

        // Создается опция --color=CLR, где CLR - одно из значений перечисления.
        // Перечисления задаются строкой, в которой '|' - задаёт альтернативы, а
        //                                          '/' - позволяет задавать алиасы
        // Для обработки таких enum-значений
        else if (opt.setParam("CLR", 0, "no/none/file|"
                                        "ansi/term|"
                                        #if defined(WIN32) || defined(_WIN32)
                                        "win32/win/windows/cmd/console"
                                        #endif
                             )
              || opt.setInitial(-1) || opt.isOption("color")
              || opt.setDescription("Force set console output coloring")
              // ", can be:\nno, none, file - disable coloring\nansi, term - set ansi terminal coloring\nwin32, win, windows, cmd, console - windows console specific coloring method"
              )
        {
            if (argsParser.hasHelpOption) return 0;

            umba::term::ConsoleType res;
            auto mapper = [](int i) -> umba::term::ConsoleType
                          {
                              switch(i)
                              {
                                  case 0 : return umba::term::ConsoleType::file;
                                  case 1 : return umba::term::ConsoleType::ansi_terminal;
                                  case 2 : return umba::term::ConsoleType::windows_console;
                                  default: return umba::term::ConsoleType::file;
                              };
                          };
            if (!opt.getParamValue( res, errMsg, mapper ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            coutWriter.forceSetConsoleType(res);
            cerrWriter.forceSetConsoleType(res);
        }


    Длинная опция
    Если первый символ '!' - internal option - недокументированная опция, обрабатывается,
    но не выводится в генерируемой справке

    Имя параметра
    Если имя параметра начинается с символа '?' - это опциональный параметр, может быть опущен.

    umba::command_line::CommandLineOption также следующие методы для задания имени,
    типа, деф значения и диапазона значений:


    // Задаёт имя параметра (для отображения по --help)
    // без задания типа и/или дефолтного значения
    bool setParam( const std::string &p )
    bool setParam( const char *p )

    // Задаем тип опции без дефолтного параметра
    bool setParam( const std::string &p, OptionType optType )

    // Не уверен, вроде добавляет булевское дефолтное значение
    bool setParam( bool defVal )


    // Опциональный параметр задается символом '?' перед именем параметра
    // Для обязательного параметра defVal не используется, но он используется для задания типа опции
    bool setParam( const std::string &p, bool defVal )

    //---

    // Задаёт параметр и int значение по умолчанию
    bool setParam( const std::string &p, int defVal )

    // Задаёт параметр и int значение по умолчанию, а также диапазон
    bool setParam( const std::string &p, int defVal, int minVal, int maxVal )

    bool setParam( const std::string &p, size_t defVal )
    bool setParam( const std::string &p, size_t defVal, size_t minVal, size_t maxVal )

    //---

    // Задаёт параметр и строковое значение по умолчанию, а также диапазон
    bool setParam( const std::string &p, std::string defVal )
    bool setParam( const std::string &p, const char* defVal )

    // Задаёт параметр/перечисление
    bool setParam( const std::string &p, int defVal, std::string enumValues ) // "one|two|three"


    // Хелперы для получения значения параметра
    bool getParamValue( bool &val, std::string &errMsg )
    bool getParamValue( int &val, std::string &errMsg )
    bool getParamValue( size_t &val, std::string &errMsg )
    bool getParamValue( std::string &val, std::string &errMsg )

    template<typename EnumType, typename EnumMapper >
    bool getParamValue( EnumType &val, std::string &errMsg, const EnumMapper &mapper )


 */



// umba::command_line::
namespace umba{
namespace command_line{



inline
bool isComment( std::string line )
{
    umba::string_plus::trim(line);

    return umba::string_plus::starts_with( line, ";" ) || umba::string_plus::starts_with( line, "#" ) || umba::string_plus::starts_with( line, "//" );
}

inline
bool isComment( std::wstring line )
{
    umba::string_plus::trim(line);

    return umba::string_plus::starts_with( line, L";" ) || umba::string_plus::starts_with( line, L"#" ) || umba::string_plus::starts_with( line, L"//" );
}


//bool startsWithAndStrip( std::string &str, const std::string &prefix );

// #include "fsUtils.h"
// #include "prog.h"
// #include "isa.h"


enum class PrintHelpStyle
{
    normal,
    wiki,
    md,
    bash_complete,
    clink_complete
};


inline
bool readOptionsFile( std::istream &optFile, std::vector<std::string> &resVec )
{
    std::string optLine;
    while( std::getline( optFile, optLine) )
    {
        umba::string_plus::trim(optLine);

        if (optLine.empty())
            continue;

        if (isComment( optLine ))
            continue;

        resVec.push_back(optLine);

    }

    return true;
}

inline
bool readOptionsFile( std::istream &optFile, std::vector<std::wstring> &resVec )
{
    std::string optLine;
    while( std::getline( optFile, optLine) )
    {
        umba::string_plus::trim(optLine);

        if (optLine.empty())
            continue;

        if (isComment( optLine ))
            continue;

        resVec.push_back(fromUtf8(optLine));

    }

    return true;
}

inline
bool readOptionsFile( const std::string &fileName, std::vector<std::string> &resVec )
{
    std::string fileDataStr;

    // Предполагается, что fileName в формате UTF8

#if defined(WIN32) || defined(_WIN32)

    if (!filesys::readFile(fileName, fileDataStr))
        return false;

#else

    if (!filesys::readFile(fileName, fileDataStr))
        return false;

#endif

#if !defined(UMBA_DISABLE_AUTO_ENCODING)

    fileDataStr = encoding::toUnicodeAuto(fileDataStr);

#endif
  
    // std::ifstream optFile(fileName.c_str());
    // if (!optFile)
    //     return false; // resVec;

    std::istringstream iss(fileDataStr);
    return readOptionsFile( iss, resVec );
}

inline
bool readOptionsFile( const std::wstring &fileName, std::vector<std::wstring> &resVec )
{
    std::string fileDataStr;

#if defined(WIN32) || defined(_WIN32)

    if (!filesys::readFile(fileName, fileDataStr))
        return false;

#else

    if (!filesys::readFile(fileName, fileDataStr))
        return false;

#endif

    // std::ifstream optFile(toUtf8(fileName).c_str());
    // if (!optFile)
    //     return false; // resVec;

    std::istringstream iss(fileDataStr);
    return readOptionsFile( iss, resVec );
}

inline
bool readBinaryFile( std::ifstream &input, std::string &data )
{
    char buf[4096];

    while( input.read( buf, sizeof(buf)) )
    {
        size_t rdSz = (size_t)input.gcount();
        data.append(buf, (size_t)rdSz);
    }

    size_t rdSz = (size_t)input.gcount();
    if (rdSz!=0  /* && rdSz!=(size_t)-1 */ )
    {
        data.append(buf, (size_t)rdSz);
    }

    return true;
}

inline
bool readBinaryFile( const std::string &fileName, std::string &data )
{
    std::ifstream input( fileName, std::ios_base::in | std::ios_base::binary );
    if (!input)
    {
        return false;
    }

    return readBinaryFile( input, data );
}

//-----------------------------------------------------------------------------
inline
bool isValidOptionNameChar(char ch)
{
    if (ch>='a' && ch<='z')
        return true;

    if (ch>='A' && ch<='Z')
        return true;

    if (ch>='0' && ch<='9')
        return true;

    if (ch=='-' || ch=='_')
        return true;

    return false;
}

//-----------------------------------------------------------------------------
inline
bool splitOptionString(const std::string &opt, std::string &optName, std::string &optVal, char &chSep)
{
    std::size_t pos = 0;
    for(; pos!=opt.size(); ++pos)
    {
        if (isValidOptionNameChar(opt[pos]))
            continue;

        auto tailStart = pos;
        if (opt[pos]==':' || opt[pos]=='=')
        {
            chSep = opt[pos];
            tailStart += 1;
        }
        else
        {
            chSep = 0;
        }

        optName = std::string(opt, 0, pos);
        optVal  = std::string(opt, tailStart, opt.size()-tailStart);

        return (opt[pos]==':' || opt[pos]=='=') ? true : false;
    }

    optName = opt;
    chSep   = 0;

    return false;
}


#include "umba/warnings/push_disable_spectre_mitigation.h"
inline
std::vector<std::string> prepareArgs( int argc, char **argv )
{
    UMBA_PROGRAM_LOCATION_INIT(argc, argv);

    std::vector<std::string> resVec;

    for( int i = 1; i<argc; ++i)
    {
        std::string argStr = argv[i];

        #if !defined(UMBA_DISABLE_AUTO_ENCODING) && !defined(UMBA_APP_MAIN)
            argStr = encoding::toUnicodeFromConsole(argStr);
        #endif

        if (argStr.empty())
           continue;

        if (argStr[0]!='-') // not an option at all
        {
            resVec.push_back(argStr);
            continue;
        }

        // Short option detected, may be it is long option

        if (argStr.size()<2) // exact one char len
        {
            resVec.push_back(argStr);
            continue;
        }

        if (argStr[1]=='-') // long option '--'
        {
            resVec.push_back(argStr);
            continue;
        }

        // Пока просто отключили возможность ввода коротких опций пакетом
        resVec.push_back(argStr);

        // // Here we got an arg in form '-X...'
        // std::string::size_type pos = 1, sz = argStr.size(); // sz 2 or more
        // for(; pos!=sz; ++pos)
        // {
        //     auto nextPos = pos+1;
        //     if (nextPos!=sz && (argStr[nextPos]=='=' || argStr[nextPos]==':'))
        //     {
        //         resVec.push_back(std::string("-") + std::string( argStr, pos ));
        //         break;
        //     }
        //     else if (nextPos!=sz && (argStr[nextPos]=='+' || argStr[nextPos]=='-'))
        //     {
        //         resVec.push_back(std::string("-") + std::string( argStr, pos, 2 ));
        //         pos = nextPos;
        //     }
        //     else
        //     {
        //         resVec.push_back( std::string("-") + std::string( argStr, pos, 1 ));
        //     }
        // }
    }

    return resVec;
}
#include "umba/warnings/pop.h"

//-----------------------------------------------------------------------------
inline
bool isCommandLineOption( std::string cmdLineArg, std::string &opt, std::string &optArg, bool &bShort, char *pFoundSepChar )
{
    bool hasSep = false;
    char sepChar = 0;

    if (umba::string_plus::starts_with_and_strip( cmdLineArg, "--"))
    {
        bShort = false;
        //bool hasEq = umba::string_plus::split_to_pair(cmdLineArg, opt, optArg, '=');
        hasSep = splitOptionString(cmdLineArg, opt, optArg, sepChar);
        umba::string_plus::trim(opt);
        umba::string_plus::trim(optArg);

        if ( !hasSep /* hasEq */  /* optArg.empty() */ )
        {
            if (umba::string_plus::ends_with_and_strip( opt, "-"))
                optArg = "-";
            else if (umba::string_plus::ends_with_and_strip( opt, "+"))
                optArg = "+";
        }

        if (pFoundSepChar)
           *pFoundSepChar = sepChar;

        return true;
    }


    if (umba::string_plus::starts_with_and_strip( cmdLineArg, "-"))
    {
        bShort = true;
        hasSep = splitOptionString(cmdLineArg, opt, optArg, sepChar);
        //bool hasEq = umba::string_plus::split_to_pair(cmdLineArg, opt, optArg, '=');
        umba::string_plus::trim(opt);
        umba::string_plus::trim(optArg);

        if ( !hasSep /* optArg.empty() */ )
        {
            if (umba::string_plus::ends_with_and_strip( opt, "-"))
                optArg = "-";
            else if (umba::string_plus::ends_with_and_strip( opt, "+"))
                optArg = "+";
        }

        if (pFoundSepChar)
           *pFoundSepChar = sepChar;

        return true;
    }

    return false;
}

#if 0
inline
bool isCommandLineOption( std::string opt, bool bShort, std::string optLongName )
{
    if (bShort==false)
    {
        if (opt==optLongName)
            return true;
        return false;
    }

    return false;
}

inline
bool isCommandLineOption( std::string opt, bool bShort, std::string optLongName, char optShort1 )
{
    if (bShort==false)
    {
        if (opt==optLongName)
            return true;
        return false;
    }

    if (opt==std::string(1,optShort1))
        return true;

    return false;
}

inline
bool isCommandLineOption( std::string opt, bool bShort, std::string optLongName, char optShort1, char optShort2 )
{
    if (bShort==false)
    {
        if (opt==optLongName)
            return true;
        return false;
    }

    if (opt==std::string(1,optShort1))
        return true;

    if (opt==std::string(1,optShort2))
        return true;

    return false;
}
#endif
//-----------------------------------------------------------------------------
inline
bool parseInteger ( const std::string &str, uint64_t &u, int ss = 10 )
{
    int64_t res = 0;

    bool bres = umba::parse_utils::toNumberEx( str.data(), str.size(), ss, &res );

    if (bres)
        u = (uint64_t)res;

    return bres;
}

//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
enum class OptionType
{
    optUnknown          ,

    optFlag             ,
    optInt              ,
    optString           ,
    optEnum             ,

    optIntList          ,
    optStringList       ,
    optStringPairList   ,
    optHeadedStringList , // eg name:val1,val2...

    optEnumList


};



struct CommandLineOptionEnumInfo
{
    std::set<std::string> valueNames;
    int value = 0;
};


struct CommandLineOptionInfo
{
    std::vector< std::string > optNames;
    bool                       internalOption = false;
    std::string                param;
    bool                       paramOptional  = false;
    std::string                description;

    OptionType                 optType;

    bool                       hasDefVal;
    bool                       hasInitVal;
    bool                       hasMinMax;

    bool                       boolDefVal;
    int                        intDefVal;    // also enum defVal
    std::string                stringDefVal;

    bool                       boolInitVal;
    int                        intInitVal;    // also enum defVal
    std::string                stringInitVal;

    int                        minVal;
    int                        maxVal;

    std::map<std::string, size_t>             enumValsAll; // second - index in enums
    std::vector< CommandLineOptionEnumInfo >  enums;


    void reset()
    {
        optNames.clear();
        internalOption = false;
        param.clear();
        paramOptional  = false;
        description.clear();

        optType = OptionType::optUnknown;

        hasDefVal  = false;
        hasInitVal = false;
        hasMinMax  = false;

        boolInitVal  = false;
        intInitVal   = 0;    // also enum defVal
        stringInitVal.clear();

        enumValsAll.clear();
        enums.clear();
    }

    int getEnumValueByName( std::string name ) const
    {
        //name = toLower(name);
        umba::string_plus::tolower(name);
        std::map<std::string, size_t>::const_iterator eit = enumValsAll.find(name);
        if (eit==enumValsAll.end())
            return -1;

        if ( eit->second >= enums.size() )
            return -1;

        return enums[eit->second].value;
    }

    void parseEnumConfigString( std::string enumValues )
    {
        //enumValues = umba::string_plus::tolower_copy(enumValues);
        umba::string_plus::tolower(enumValues);

        int enumVal = 0;
        std::vector< std::string > namesVec;
        // splitToVector( enumValues, namesVec, '|' );
        namesVec = umba::string_plus::split(enumValues, '|');

        for( auto nameAlters : namesVec )
        {
            umba::string_plus::trim(nameAlters);
            if (nameAlters.empty())
                continue;

            std::vector< std::string > altersVec = umba::string_plus::split(nameAlters, '/');
            //splitToVector( nameAlters, altersVec, '/' );

            CommandLineOptionEnumInfo ei;
            ei.value = enumVal;

            size_t idx = (size_t)enums.size();

            for( auto na : altersVec )
            {
                umba::string_plus::trim(na);
                if (na.empty())
                   continue;

                ei.valueNames.insert(na);
                enumValsAll[na] = idx;

            }
            //ei.valueNames.insert( altersVec.begin(), altersVec.end() );
            enums.push_back(ei);

            ++enumVal;
        }
    }


    std::string getAllEnumNames( const std::string &sep
                               , const std::string &sepLast
                               , const std::string &alter
                               , const std::string &before = std::string()
                               , const std::string &after = std::string()
                               ) const
    {
        std::string res;

        if (enums.empty())
            return res;

        std::vector< CommandLineOptionEnumInfo >::const_iterator eit = enums.begin();
        std::vector< CommandLineOptionEnumInfo >::const_iterator last = enums.end();
        --last;

        for(; eit!=enums.end(); ++eit )
        {
            std::string curEnumAltersStr;
            const std::set<std::string> &valueNames = eit->valueNames;
            for( const auto& en : valueNames)
            {
                if (!curEnumAltersStr.empty())
                    curEnumAltersStr += alter;
                curEnumAltersStr += before + en + after;
            }

            if (!res.empty())
            {
                if (eit==last)
                    res += sepLast;
                else
                    res += sep;
            }

            //res += before + curEnumAltersStr + after;
            res += curEnumAltersStr;
        }

        return res;

    }

    std::string formatNumberHelper( int i ) const
    {
        UMBA_USED(i); //!!!
        char buf[128];
        size_t numCharsOut = umba::format_utils::formatNumber( &buf[0], 128, (int64_t)intDefVal, 10 );
        buf[numCharsOut] = 0;
        return std::string(buf);
    }

    std::string formatNumberHelper( unsigned u ) const
    {
        UMBA_USED(u); //!!!
        char buf[128];
        size_t numCharsOut = umba::format_utils::formatNumber( &buf[0], 128, (uint64_t)intDefVal, 10 );
        buf[numCharsOut] = 0;
        return std::string(buf);
    }

    std::string formatNumberHelper( int64_t i ) const
    {
        UMBA_USED(i); //!!!
        char buf[128];
        size_t numCharsOut = umba::format_utils::formatNumber( &buf[0], 128, (int64_t)intDefVal, 10 );
        buf[numCharsOut] = 0;
        return std::string(buf);
    }

    std::string formatNumberHelper( uint64_t u ) const
    {
        UMBA_USED(u); //!!!
        char buf[128];
        size_t numCharsOut = umba::format_utils::formatNumber( &buf[0], 128, (uint64_t)intDefVal, 10 );
        buf[numCharsOut] = 0;
        return std::string(buf);
    }

    std::string getEnumValNameByValue( int enumValue ) const
    {
        std::vector< CommandLineOptionEnumInfo >::const_iterator eit = enums.begin();
        for(; eit!=enums.end(); ++eit )
        {
            if (eit->value==enumValue)
            {
                return * (eit->valueNames.rbegin());
            }
        }

        return std::string();
    }

    std::string getDefValString(char quotChar) const
    {
        if (!hasDefVal)
            return std::string();

        if (!paramOptional)
            return std::string();

        std::string quotString = std::string(1, quotChar);

        switch(optType)
        {
            case OptionType::optFlag  : return quotString + (boolDefVal ? "TRUE" : "FALSE") + quotString;
            case OptionType::optInt   : return quotString + formatNumberHelper(intDefVal) + quotString;
                                        //return cppHelpersFormatInt( intDefVal );
            case OptionType::optString: return quotString + stringDefVal + quotString;
            case OptionType::optEnum  :
            {
                /*
                std::map<std::string, int>::const_iterator it = enumVals.begin();
                for(; it != enumVals.end(); ++it)
                {
                    if (it->second==intDefVal)
                        return std::string("'") + it->first + std::string("'");
                }
                */

                auto res = getEnumValNameByValue(intDefVal);
                if (res.empty())
                    return quotString + "<AUTO>" + quotString;
                return quotString + res + quotString;
            }

            case OptionType::optUnknown:
            case OptionType::optIntList:
            case OptionType::optStringList:
            case OptionType::optStringPairList:
            case OptionType::optHeadedStringList:
            case OptionType::optEnumList:
                [[fallthrough]];
            //case OptionType::optUnknown:
            default: {}
        }

        return std::string();
    }

    std::string getInitialValString(char quotChar) const
    {
        if (!hasInitVal)
            return std::string();

        std::string quotStr = std::string(1, quotChar);


        switch(optType)
        {
            case OptionType::optFlag  : return quotStr + (boolInitVal ? "TRUE" : "FALSE") + quotStr;
            case OptionType::optInt   : return quotStr + formatNumberHelper(intDefVal) + quotStr;
                                        //return cppHelpersFormatInt( intInitVal );
            case OptionType::optString: return quotStr + stringInitVal + quotStr;
            case OptionType::optEnum  :
            {
                /*
                std::map<std::string, int>::const_iterator it = enumVals.begin();
                for(; it != enumVals.end(); ++it)
                {
                    if (it->second==intInitVal)
                        return std::string("'") + it->first + std::string("'");
                }

                return "<AUTO>";
                */

                auto res = getEnumValNameByValue(intInitVal);
                if (res.empty())
                    return quotStr + "<AUTO>" + quotStr;
                return quotStr + res + quotStr;
            }

            case OptionType::optUnknown:
            case OptionType::optIntList:
            case OptionType::optStringList:
            case OptionType::optStringPairList  :
            case OptionType::optHeadedStringList:
            case OptionType::optEnumList:
                [[fallthrough]];
                //case OptionType::optUnknown:
            default: {}
        }

        return std::string();
    }


    std::string getAllOptionNames( const std::string &sep="/" ) const
    {
        //return mergeStrings( optNames, sep );
        std::string res;

        for( const auto & s : optNames )
        {
            if (!res.empty())
               res.append( sep );

            if (s.size()==1)
                res.append( std::string("-") + s );
            else
                res.append( std::string("--") + s );
        }

        return res;
    }

};


#include "umba/warnings/interface_begin.h"
struct ICommandLineOptionCollector
{

    virtual ~ICommandLineOptionCollector() {}

    virtual void ignoreOptInfo( ) = 0;
    virtual void setCollectMode( bool bCollect ) = 0;

    virtual void addOption( std::string optLongName ) = 0;
    virtual void addOption( char optShort ) = 0;

    //virtual const CommandLineOptionInfo& getLastAddedOptionInfo() = 0;
    virtual const CommandLineOptionInfo& getCurrentOptionInfo() = 0;

    virtual const std::set<std::string> * getTakenOpts() = 0;
    virtual void  addTakenOpt( const std::string &optName ) = 0;

    virtual void setOptionInitial( bool v ) = 0;
    virtual void setOptionInitial( int  v  ) = 0;
    virtual void setOptionInitial( const std::string &v ) = 0;

    virtual void setOptionParam( const std::string &param ) = 0;
    virtual void setOptionParam( bool defVal ) = 0;
    virtual void setOptionParam( const std::string &param, OptionType optType ) = 0;
    virtual void setOptionParam( const std::string &param, bool defVal ) = 0;
    virtual void setOptionParam( const std::string &param, int defVal  ) = 0;
    virtual void setOptionParam( const std::string &param, int defVal, int minVal, int maxVal ) = 0;
    virtual void setOptionParam( const std::string &param, const std::string &defVal ) = 0;
    virtual void setOptionParam( const std::string &param, int defVal, std::string enumValues ) = 0;

    virtual void addOptionDescription( const std::string &d ) = 0;
    virtual void resetCurCollectedOption( ) = 0;

    virtual bool isNormalPrintHelpStyle() const = 0;
    virtual void setPrintHelpStyle( PrintHelpStyle printHelpStyle ) = 0;
    virtual PrintHelpStyle getPrintHelpStyle() const = 0;

    virtual std::string makeText( std::string::size_type width, PrintHelpStyle printHelpStyle, const std::set<std::string> *pHelpOnlyThats ) = 0;
    virtual std::string makeText( std::string::size_type width, PrintHelpStyle printHelpStyle, std::string *pPrefix, std::string *pSuffix, bool useExt ) = 0;
    virtual std::string makeText( std::string::size_type width, PrintHelpStyle printHelpStyle, std::string *pPrefix, std::string *pSuffix, bool useExt, const std::set<std::string> *pHelpOnlyThats ) = 0;

    virtual std::string makeText( std::string::size_type width, const std::set<std::string> *pHelpOnlyThats ) = 0;
    virtual std::string makeText( std::string::size_type width, std::string *pPrefix, std::string *pSuffix, bool useExt ) = 0;
    virtual std::string makeText( std::string::size_type width, std::string *pPrefix, std::string *pSuffix, bool useExt, const std::set<std::string> *pHelpOnlyThats ) = 0;

    virtual std::string getExeName(bool useExt) const = 0;

    //const std::set<std::string>

};
#include "umba/warnings/interface_end.h"





// Это чекер для опций
struct CommandLineOption
{
    bool          fOption;
    bool          fResponseFile;
    bool          fShort;
    std::string   argOrg;
    std::string   name;
    std::string   optArg;
    bool          valueOptional;
    char          sepChar;
    ICommandLineOptionCollector *pCollector;

    CommandLineOption( std::string a, ICommandLineOptionCollector *pCol = 0)
    : fOption(false)
    , fResponseFile(false)
    , fShort(false)
    , argOrg(a)
    , name()
    , optArg()
    , valueOptional(false)
    , sepChar(0)
    , pCollector(pCol)
    {
        fOption = isCommandLineOption( argOrg, name, optArg, fShort, &sepChar );
        if (!fOption)
        {
            name = argOrg;
            if (umba::string_plus::starts_with_and_strip( name, "@"))
                fResponseFile = true;
        }
    }

    ~CommandLineOption()
    {
        if (pCollector)
            pCollector->resetCurCollectedOption();
    }

    void resetOptionInfoCollector()
    {
        pCollector = 0;
    }

    bool hasArg() const
    {
        return !optArg.empty();
    }

    bool isResponseFile() const
    {
        return fResponseFile;
    }

    bool isOption() const
    {
        return fOption;
    }

    bool isPureHelpOption() const
    {
        if (!fOption)
            return false;

        if (fShort)
            return name=="h" || name=="?";
        else
            return name=="help";
    }

    bool isHelpOption() const
    {
        if (!fOption)
            return false;

        if (isPureHelpOption() || name=="autocomplete-install" || name=="autocomplete-uninstall")
            return true;

        return false;
    }

    bool isSomeKindOfQuet() const
    {
        if (!fOption)
            return false;

        return name=="quet" || name=="q" || name=="version" || name=="v" || name=="build-info" || name=="build-info-x";
    }

    static unsigned getBuiltinsDisableOptionMainFlag  () { return umba::program_location::BuiltinOptionsLocationFlag::appGlobal   ; }
    static unsigned getBuiltinsDisableOptionCustomFlag() { return umba::program_location::BuiltinOptionsLocationFlag::customGlobal; }
    static unsigned getBuiltinsDisableOptionUserFlag  () { return umba::program_location::BuiltinOptionsLocationFlag::userLocal   ; }

    static std::string getBuiltinsDisableOptionName( unsigned f )
    {
        using namespace umba::program_location;
        switch(f)
        {
            case BuiltinOptionsLocationFlag::appGlobal    : return "no-builtin-options";
            case BuiltinOptionsLocationFlag::customGlobal : return "no-custom-builtin-options";
            case BuiltinOptionsLocationFlag::userLocal    : return "no-user-builtin-options";
            default : return "";
        }
    }

    static std::string getBuiltinsDisableOptionMainName  () { return getBuiltinsDisableOptionName(umba::program_location::BuiltinOptionsLocationFlag::appGlobal   ); }
    static std::string getBuiltinsDisableOptionCustomName() { return getBuiltinsDisableOptionName(umba::program_location::BuiltinOptionsLocationFlag::customGlobal); }
    static std::string getBuiltinsDisableOptionUserName  () { return getBuiltinsDisableOptionName(umba::program_location::BuiltinOptionsLocationFlag::userLocal   ); }

    bool isBuiltinsDisableOptionMain  () const { return isOption( getBuiltinsDisableOptionMainName  () ); }
    bool isBuiltinsDisableOptionCustom() const { return isOption( getBuiltinsDisableOptionCustomName() ); }
    bool isBuiltinsDisableOptionUser  () const { return isOption( getBuiltinsDisableOptionUserName  () ); }


    bool isWhereOption() const
    {
        if (!fOption)
            return false;

        return name=="where";
    }

    bool isHelpStyleOption() const
    {
        if (!fOption)
            return false;

        PrintHelpStyle phs = PrintHelpStyle::normal;

        if (name=="wiki")
            phs = PrintHelpStyle::wiki;
        else if (name=="md")
            phs = PrintHelpStyle::md;
        else if (name=="bash")
            phs = PrintHelpStyle::bash_complete;
        else if (name=="clink")
            phs = PrintHelpStyle::clink_complete;
        else
            return false;

        if (pCollector)
        {
            pCollector->setPrintHelpStyle( phs );
        }

        return true;
    }

    static
    std::set<std::string> getHelpStyleOptions()
    {
        static std::set<std::string> _ = { "wiki", "md", "bash", "clink" };
        return _;
    }


/*


        else if (opt.isOption("!wiki") || opt.setDescription("Help output in Wiki format"))
        {
            printHelpStyle = PrintHelpStyle::wiki;
        }
        else if (opt.isOption("!bash") || opt.setDescription("Help output in 'bash complete' format"))
        {
            printHelpStyle = PrintHelpStyle::bash_complete;
        }
        else if (opt.isOption("!clink") || opt.setDescription("Help output in 'clink complete' format"))
*/


    std::string getHelpOptionsString() const
    {
        return
        "-h\n"
        "-?\n"
        "--help\n"
        "    Show this help.\n"
        ;

    }

    bool isOption( std::string optLongName ) const
    {
        if (pCollector)
        {
            pCollector->addOption( optLongName );
        }

        if (!fOption)
            return false;
        if (fShort)
            return false;

        //if (optLongName)
        umba::string_plus::ltrim( optLongName, umba::string_plus::is_one_of<char>(",+-?!") );
        umba::string_plus::rtrim( optLongName, umba::string_plus::is_one_of<char>(",+-") );

        return name==optLongName;
    }

    bool isOption( char optShort ) const
    {
        if (pCollector)
        {
            pCollector->addOption( optShort );
        }

        if (!fOption)
            return false;
        if (!fShort)
            return false;

        return name==std::string( 1, optShort );
    }

    bool infoIgnore( )
    {
        if (pCollector)
        {
            pCollector->ignoreOptInfo( );
        }
        return false;
    }

    bool setDescription( const std::string &d )
    {
        if (pCollector)
        {
            pCollector->addOptionDescription( d );
        }
        return false;
    }


    // setParam должен вызываться самым первым в if

    bool setInitial( bool b )
    {
        if (pCollector)
        {
            pCollector->setOptionInitial( b );
        }
        return false;
    }

    bool setInitial( int i )
    {
        if (pCollector)
        {
            pCollector->setOptionInitial( i );
        }
        return false;
    }

    bool setInitial( size_t i )
    {
        if (pCollector)
        {
            pCollector->setOptionInitial( (int)i );
        }
        return false;
    }

    bool setInitial( const std::string &s )
    {
        if (pCollector)
        {
            pCollector->setOptionInitial( s );
        }
        return false;
    }

    bool setInitial( const char *s )
    {
        if (pCollector)
        {
            if (s)
                pCollector->setOptionInitial( std::string(s) );
            else
                pCollector->setOptionInitial( std::string() );
        }
        return false;
    }

    bool setParam( const std::string &p )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p );
        }
        return false;
    }

    bool setParam( const char *p )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( std::string(p) );
        }
        return false;
    }

    // Задаем тип опции без дефолтного параметра
    bool setParam( const std::string &p, OptionType optType )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, optType );
        }
        return false;
    }

    bool setParam( bool defVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( defVal );
        }
        return false;
    }


    // Опциональный параметр задается символом '?' перед именем параметра
    // Для обязательного параметра defVal не используется, но он используется для задания типа опции

    bool setParam( const std::string &p, bool defVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, defVal );
        }
        return false;
    }

    //---

    bool setParam( const std::string &p, int defVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, defVal );
        }
        return false;
    }

    bool setParam( const std::string &p, int defVal, int minVal, int maxVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, defVal, minVal, maxVal );
        }
        return false;
    }

    bool setParam( const std::string &p, size_t defVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, (int)defVal );
        }
        return false;
    }

    bool setParam( const std::string &p, size_t defVal, size_t minVal, size_t maxVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, (int)defVal, (int)minVal, (int)maxVal );
        }
        return false;
    }

    //---

    bool setParam( const std::string &p, std::string defVal )
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, defVal );
        }
        return false;
    }

    bool setParam( const std::string &p, const char* defVal )
    {
        if (pCollector)
        {
            if (defVal)
               pCollector->setOptionParam( p, std::string(defVal) );
            else
               pCollector->setOptionParam( p, OptionType::optString );
        }
        return false;
    }

    //---

    bool setParam( const std::string &p, int defVal, std::string enumValues ) // "one|two|three"
    {
        if (pCollector)
        {
            pCollector->setOptionParam( p, defVal, enumValues );
        }
        return false;
    }

    std::string getOptionNames( std::string sep = "/" ) const
    {
        const auto &optInfo = pCollector->getCurrentOptionInfo();
        return optInfo.getAllOptionNames(sep);
    }

    // У нас уже была getOptionNames - пропустил. Ладно, будет алиас
    std::string getNames(const std::string &sep="/") const
    {
        const auto &optInfo = pCollector->getCurrentOptionInfo();
        return optInfo.getAllOptionNames(sep);
    }

    std::string getNamesParen(const std::string &sep="/") const
    {
        return "(" + getNames(sep) + ")";
    }

    std::vector<std::string> getOptionNamesVector() const
    {
        const auto &optInfo = pCollector->getCurrentOptionInfo();
        return optInfo.optNames;
    }

    bool checkCurOptionType(std::string &errMsg, OptionType optTypeCheckFor) const
    {
        if (!pCollector)
        {
            errMsg = std::string("pCollector==0 (") + name + std::string(")");
            return false;
        }

        const auto &optInfo = pCollector->getCurrentOptionInfo();

        if (optInfo.optType==OptionType::optUnknown)
        {
            errMsg = std::string("Option not configured to use getParamValue method (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }

        if (optInfo.optType!=optTypeCheckFor)
        {
            errMsg = std::string("Option configured to use getParamValue method with different type (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }

        return true;
    }

    template< typename T>
    bool getParamValueImplHelper( T &val, std::string &errMsg, T defVal, OptionType optType )
    {
        if (!checkCurOptionType(errMsg, optType))
            return false;

        const auto &optInfo = pCollector->getCurrentOptionInfo();


        // if (optInfo.optType==OptionType::optUnknown)
        // {
        //     errMsg = std::string("Option not configured to use getParamValue method (") + optInfo.getAllOptionNames("/") + std::string(")");
        //     return false;
        // }
        //  
        // if (optInfo.optType!=optType)
        // {
        //     errMsg = std::string("Option configured to use getParamValue method with different type (") + optInfo.getAllOptionNames("/") + std::string(")");
        //     return false;
        // }

        if (optArg.empty() && optInfo.optType==OptionType::optFlag)
        {
            if (optInfo.hasDefVal)
               val = defVal;
            else
               val = true;

            return true;
        }


        if (optArg.empty())
        {
            if (!optInfo.paramOptional)
            {
                errMsg = std::string("Missing mandatory value (") + optInfo.getAllOptionNames("/") + std::string("), argument: '" + argOrg + "'");
                return false;
            }

            if (!optInfo.hasDefVal)
            {
                errMsg = std::string("Missing mandatory value, and there is no default value taken (") + optInfo.getAllOptionNames("/") + std::string(")");
                return false;
            }

            val = defVal;

            return true;
        }
        else
        {
            // val = optArg;
            // return true;
        }

        return false;

    }

    bool getParamValue( bool &val, std::string &errMsg )
    {
        if (!pCollector)
        {
            errMsg = std::string("pCollector==0 (") + name + std::string(")");
            return false;
        }

        const auto &optInfo = pCollector->getCurrentOptionInfo();

        if (getParamValueImplHelper( val, errMsg, optInfo.boolDefVal, OptionType::optFlag ))
            return true; // readed ok

        if (!errMsg.empty())
            return false; // get value failed

        // need parsing
        std::string optArgLower = umba::string_plus::tolower_copy(optArg);

        if (optArgLower=="-" || optArgLower=="0" || optArgLower=="false" || optArgLower=="f" || optArgLower=="no" || optArgLower == "n")
        {
            val = false;
            return true;
        }
        else if (optArgLower=="+" || optArgLower=="1" || optArgLower=="true" || optArgLower=="t" || optArgLower=="yes" || optArgLower == "y")
        {
            val = true;
            return true;
        }

        errMsg = std::string("Invalid option value taken (") + optInfo.getAllOptionNames("/") + std::string(")");
        return false;

    }

    bool getParamValue( int &val, std::string &errMsg )
    {
        if (!pCollector)
        {
            errMsg = std::string("pCollector==0 (") + name + std::string(")");
            return false;
        }

        const auto &optInfo = pCollector->getCurrentOptionInfo();

        if (getParamValueImplHelper( val, errMsg, optInfo.intDefVal, OptionType::optInt ))
            return true; // readed ok

        if (!errMsg.empty())
            return false; // get value failed

        // need parsing

        bool neg = false;
        std::string optArgCopy = optArg;
        if (!optArgCopy.empty() && (optArgCopy[0]=='+' || optArgCopy[0]=='-'))
        {
            if (optArgCopy[0]=='-')
                neg = true;
            optArgCopy.erase( 0, 1 );
        }

        uint64_t v = 0;
        if (!parseInteger(optArgCopy, v))
        {
            errMsg = std::string("Invalid option value taken (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }

        val = (int)v;
        if (neg)
            val = -val;

        if (!optInfo.hasMinMax)
           return true;

        if ( (val < optInfo.minVal) || (val > optInfo.maxVal) )
        {
            errMsg = std::string("Option value is out of range (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }

        return true;
    }

    bool getParamValue( size_t &val, std::string &errMsg )
    {
        int i;
        if (!getParamValue( i, errMsg ))
            return false;

        const auto &optInfo = pCollector->getCurrentOptionInfo();

        if (i<0)
        {
            errMsg = std::string("Option value is invalid - negative value taken for unsigned option (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }

        val = (size_t)i;

        return true;
    }


    bool getParamValue( std::string &val, std::string &errMsg )
    {
        if (!checkCurOptionType(errMsg, OptionType::optString))
            return false;

        const auto &optInfo = pCollector->getCurrentOptionInfo();

        if (getParamValueImplHelper( val, errMsg, optInfo.stringDefVal, OptionType::optString ))
            return true; // readed ok

        if (!errMsg.empty())
            return false; // get value failed

        // need parsing

        val.clear();

        val = optArg;

        return true;
    }

    bool getParamValue( std::vector<std::string> &strVec, std::string &errMsg, const std::string &listSeps=",;" )
    {
        if (!checkCurOptionType(errMsg, OptionType::optStringList))
            return false;

        strVec = parse_utils::optionStringSplitToVector(optArg, listSeps);

        return true;
    }

    bool getParamValue( std::vector< std::pair<std::string, std::string> > &strPairsVec, std::string &errMsg, const std::string &listSeps=",;", const std::string &pairSeps=":=" )
    {
        if (!checkCurOptionType(errMsg, OptionType::optStringPairList))
            return false;

        strPairsVec.clear();

        auto strVec = parse_utils::optionStringSplitToVector(optArg, listSeps);

        for(auto && str: strVec)
        {
            std::string f, s;
            parse_utils::optionStringSplitToPair(str, f, s, pairSeps);
            strPairsVec.emplace_back(f, s);
        }

        return true;
    }

    bool getParamValue( std::string &head, std::vector<std::string> &strVec, std::string &errMsg, const std::string &listSeps=",;", const std::string &headSeps=":=" )
    {
        if (!checkCurOptionType(errMsg, OptionType::optHeadedStringList))
            return false;

        std::string str;
        parse_utils::optionStringSplitToPair(optArg, head, str, headSeps);

        strVec = parse_utils::optionStringSplitToVector(str, listSeps);

        return true;
    }


    #include "umba/warnings/push_disable_spectre_mitigation.h"
    template<typename EnumType, typename EnumMapper >
    bool getParamValue( EnumType &val, std::string &errMsg, const EnumMapper &mapper )
    {
        if (!pCollector)
        {
            errMsg = std::string("pCollector==0 (") + name + std::string(")");
            return false;
        }

        const auto &optInfo = pCollector->getCurrentOptionInfo();

        int intVal;
        if (getParamValueImplHelper( intVal, errMsg, optInfo.intDefVal, OptionType::optEnum ))
        {
            val = mapper(intVal);
            return true; // readed ok
        }

        if (!errMsg.empty())
            return false; // get value failed

        // need parsing

        //std::string optArgCopy = toLower(optArg);

        /*
        !!! Тут закоменчено что-то старое, хз зачем я тут внимание акцентировал.
        std::map<std::string, int>::const_iterator eit = optInfo.enumVals.find(optArgCopy);
        if (eit == optInfo.enumVals.end())
        {
            errMsg = std::string("Invalid option value taken. Option value can be one of: ") + optInfo.getAllEnumNames(", ", " or ") + std::string(" (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }
        */

        int enumIntVal = optInfo.getEnumValueByName( optArg );
        if (enumIntVal<0)
        {
            errMsg = std::string("Invalid option value taken. Option value can be one of: ")
                   + optInfo.getAllEnumNames(", ", " or ", "/") + std::string(" (") + optInfo.getAllOptionNames("/") + std::string(")");
            return false;
        }

        val = mapper(enumIntVal);

        return true;
    }
    #include "umba/warnings/pop.h"


/*
    std::string getAllOptionNames( const std::string &sep ) const
    std::string getAllEnumNames( const std::string &sep, const std::string &sepLast ) const

*/
    //virtual const CommandLineOptionInfo& getCurrentOptionInfo() = 0;

}; // struct CommandLineOption

/*
enum class OptionType
{
    optUnknown     ,

    optFlag        ,
    optInt         ,
    optString      ,
    optEnum        ,

    optIntList     ,
    optStringList  ,
    optEnumList
};



struct CommandLineOptionInfo
{
    std::vector< std::string > optNames;
    bool                       internalOption = false;
    std::string                param;
    bool                       paramOptional  = false;
    std::string                description;

    OptionType                 optType;

    bool                       hasDefVal;
    bool                       hasMinMax;

    bool                       boolDefVal;
    int                        intDefVal;    // also enum defVal
    std::string                stringDefVal;

    int                        minVal;
    int                        maxVal;

    std::map<std::string, int> enumVals;

*/




// В наборе для проверки на уникальность храним все ключики - мапа ключ->опция
// Опция хранится как объект

// #include "umba/warnings/interface_begin.h"
class CommandLineOptionCollectorImplBase : public ICommandLineOptionCollector
{
protected:

    bool collectOptionsInfo = false;
    bool flagFailed = false;
    bool flagIgnore = false;
    CommandLineOptionInfo                 curOptInfo;
    std::vector<CommandLineOptionInfo>    optionsList;

    std::set<std::string>                 usedOpts;

    std::set<std::string>                 takenOpts;

    PrintHelpStyle                        m_printHelpStyle = PrintHelpStyle::normal;

    virtual void onOptionDup( const std::string &opt ) = 0;

public:

    UMBA_RULE_OF_FIVE(CommandLineOptionCollectorImplBase, default, default, default, default, default);

    virtual bool isNormalPrintHelpStyle() const override
    {
        return m_printHelpStyle == PrintHelpStyle::normal;
    }
    //void setPrintHelpStyle( PrintHelpStyle printHelpStyle ) = 0;
    virtual void setPrintHelpStyle( PrintHelpStyle phs ) override
    {
        m_printHelpStyle = phs;
    }

    virtual PrintHelpStyle getPrintHelpStyle() const override
    {
        return m_printHelpStyle;
    }

    bool isFailed()
    {
        return flagFailed;
    }

    virtual const std::set<std::string> * getTakenOpts() override
    {
        return &takenOpts;
    }

    virtual void  addTakenOpt( const std::string &optName ) override
    {
        takenOpts.insert( optName );
    }


    virtual void setCollectMode( bool bCollect ) override
    {
        collectOptionsInfo = bCollect;
    }

    virtual void ignoreOptInfo( ) override
    {
        flagIgnore = true;
    }

    /*
    virtual const CommandLineOptionInfo& getLastAddedOptionInfo() override
    {
        return optionsList.back();
    }
    */

    virtual const CommandLineOptionInfo& getCurrentOptionInfo() override
    {
        return curOptInfo;
    }

    virtual void addOption( std::string optLongName ) override
    {
        //bool internalOption = false;
        if (!optLongName.empty() && optLongName[0]=='!')
        {
            optLongName.erase(0,1);
            curOptInfo.internalOption = true;
        }
        //internalOption
        curOptInfo.optNames.push_back(optLongName);

        if (!flagIgnore && collectOptionsInfo)
        {
            if (usedOpts.find(optLongName)!=usedOpts.end())
            {
                flagFailed = true;
                onOptionDup( optLongName );
            }
            usedOpts.insert(optLongName);
        }
    }

    virtual void addOption( char optShort ) override
    {
        addOption( std::string( 1, optShort ) );
    }

    virtual void setOptionInitial( bool v ) override
    {
        curOptInfo.hasInitVal  = true;
        curOptInfo.boolInitVal = v;
    }

    virtual void setOptionInitial( int  v  ) override
    {
        curOptInfo.hasInitVal = true;
        curOptInfo.intInitVal = v;
    }

    virtual void setOptionInitial( const std::string &v ) override
    {
        curOptInfo.hasInitVal     = true;
        curOptInfo.stringInitVal  = v;
    }


    virtual void setOptionParam( const std::string &param ) override
    {
        curOptInfo.param = param;
        if (!curOptInfo.param.empty() && curOptInfo.param[0]=='?')
        {
            curOptInfo.param.erase(0, 1);
            curOptInfo.paramOptional = true;
        }
    }

    virtual void setOptionParam( const std::string &param, OptionType optType ) override
    {
        setOptionParam(param);
        curOptInfo.optType = optType;
    }

    virtual void setOptionParam( bool defVal ) override
    {
        curOptInfo.optType = OptionType::optFlag;
        curOptInfo.hasDefVal = true;
        curOptInfo.boolDefVal = defVal;
    }

    virtual void setOptionParam( const std::string &param, bool defVal ) override
    {
        setOptionParam(param);
        curOptInfo.optType = OptionType::optFlag;

        curOptInfo.hasDefVal = true;
        curOptInfo.boolDefVal = defVal;
    }

    virtual void setOptionParam( const std::string &param, int defVal  ) override
    {
        setOptionParam(param);
        curOptInfo.optType = OptionType::optInt;

        curOptInfo.hasDefVal = true;
        curOptInfo.intDefVal = defVal;
    }

    virtual void setOptionParam( const std::string &param, int defVal, int minVal, int maxVal ) override
    {
        setOptionParam(param);
        curOptInfo.optType = OptionType::optInt;

        curOptInfo.hasDefVal = true;
        curOptInfo.intDefVal = defVal;

        curOptInfo.hasMinMax = true;
        curOptInfo.minVal = minVal;
        curOptInfo.maxVal = maxVal;
    }

    virtual void setOptionParam( const std::string &param, const std::string &defVal ) override
    {
        setOptionParam(param);
        curOptInfo.optType = OptionType::optString;

        curOptInfo.hasDefVal = true;
        curOptInfo.stringDefVal = defVal;
    }

    virtual void setOptionParam( const std::string &param, int defVal, std::string enumValues ) override
    {
        setOptionParam(param);
        curOptInfo.optType = OptionType::optEnum;

        curOptInfo.hasDefVal = true;
        curOptInfo.intDefVal = defVal;

        /*
        enumValues = toLower(enumValues);

        std::vector< std::string > vec;
        splitToVector( enumValues, vec, '|' );

        for( int i=0; i<(int)vec.size(); ++i)
        {
            curOptInfo.enumVals[ vec[i] ] = i;
        }

        //std::map<std::string, int> enumVals;
        */

        curOptInfo.parseEnumConfigString(enumValues);
    }

/*
enum class OptionType
{
    optUnknown     ,

    optFlag        ,
    optInt         ,
    optString      ,
    optEnum        ,

    optIntList     ,
    optStringList  ,
    optEnumList
};


struct CommandLineOptionInfo
{
    std::vector< std::string > optNames;
    bool                       internalOption = false;
    std::string                param;
    bool                       paramOptional  = false;
    std::string                description;

    OptionType                 optType;

    bool                       hasDefVal;
    bool                       hasMinMax;

    bool                       boolDefVal;
    int                        intDefVal;    // also enum defVal
    std::string                stringDefVal;

    int                        minVal;
    int                        maxVal;

    std::map<std::string, int> enumVals;

    void reset()
    {
        optNames.clear();
        internalOption = false;
        param.clear();
        paramOptional  = false;
        description.clear();

        optType = OptionType::optUnknown;

        hasDefVal = false;
        hasMinMax = false;
    }
};

*/

    virtual void addOptionDescription( const std::string &d ) override
    {
        curOptInfo.description = d;
        if (!curOptInfo.optNames.empty() && !flagIgnore && collectOptionsInfo)
        {
            std::sort( curOptInfo.optNames.begin(), curOptInfo.optNames.end(), umba::text_utils::StringsLessFirstBySize(true) ); // StringsLess
            optionsList.push_back( curOptInfo );
        }
        curOptInfo.reset();
        flagIgnore = false;
    }

    virtual void resetCurCollectedOption( ) override
    {
        curOptInfo.reset();
    }

    virtual std::string makeText( std::string::size_type width, const std::set<std::string> *pHelpOnlyThats ) override
    {
        return makeText( width, m_printHelpStyle, pHelpOnlyThats );
    }

    virtual std::string makeText( std::string::size_type width, std::string *pPrefix, std::string *pSuffix, bool useExt ) override
    {
        return makeText( width, m_printHelpStyle, pPrefix, pSuffix, useExt );
    }

    virtual std::string makeText( std::string::size_type width, std::string *pPrefix, std::string *pSuffix, bool useExt, const std::set<std::string> *pHelpOnlyThats ) override
    {
        return makeText( width, m_printHelpStyle, pPrefix, pSuffix, useExt, pHelpOnlyThats );
    }

    char getHelpQuotChar() const
    {
        return (getPrintHelpStyle()==PrintHelpStyle::md || getPrintHelpStyle()==PrintHelpStyle::wiki) ? '`' : '\'';
    }

    std::string getHelpQuotString() const
    {
        return std::string(1, getHelpQuotChar());
    }

    virtual std::string getExeName(bool useExt) const override
    {
        std::string exeName = umba::program_location::getExeName<std::string>();
        //std::string exeVarName  ;

        if (useExt)
        {
            exeName = umba::filename::getFileName(exeName); // umba::filename::appendExtention(umba::filename::getFileName(exeName), umba::filename::getFileExtention(exeName) );
        }
        else
        {
           exeName = umba::filename::getName(exeName);
        }

        return exeName;
    }

    virtual std::string makeText( std::string::size_type width, PrintHelpStyle style, std::string *pPrefix, std::string *pSuffix, bool useExt, const std::set<std::string> *pHelpOnlyThats ) override
    {
        std::stringstream oss;

        std::string exeFullName = getExeName(useExt);
        std::string exeVarName;
        // std::string exeFullName = umba::program_location::getExeName<std::string>();
        // std::string exeVarName  ;
        //  
        // if (useExt)
        // {
        //    exeFullName = umba::filename::appendExtention(umba::filename::getFileName(exeFullName), umba::filename::getFileExtention(exeFullName) );
        // }
        // else
        // {
        //    exeFullName = umba::filename::getFileName(exeFullName);
        // }

        exeVarName = //std::string("_") +
                     //makeCppName( exeFullName );
            exeFullName; // !!!

        std::string prefix;
        std::string suffix;

        if (style==PrintHelpStyle::bash_complete)
        {
            // complete -W "-a -b c" x
            prefix = "complete -W \"";
            suffix = std::string("\" ") + exeFullName;
            oss<<prefix;
        }
        else if (style==PrintHelpStyle::clink_complete)
        {
            prefix = std::string("local ") + exeVarName + std::string("_tree = { ");
            suffix = std::string(" } clink.arg.register_parser(\"") + exeFullName + std::string("\", ") + exeVarName + std::string("_tree)");
            oss<<prefix;
        }

        if (pPrefix)
           *pPrefix = prefix;

        if (pSuffix)
           *pSuffix = suffix;


        size_t totalNamesPrinted = 0;
        for( const auto &optInfo : optionsList )
        {
            if (optInfo.internalOption)
                continue;

            bool skipThisOption = false;

            if (pHelpOnlyThats && !pHelpOnlyThats->empty())
            {
                bool optionFound = false;
                size_t i = 0, sz = optInfo.optNames.size();
                //for( const auto &optName : optInfo.optNames )
                for(; i!=sz; ++i)
                {
                    const auto &optName = optInfo.optNames[i];
                    if (pHelpOnlyThats->find(optName)!=pHelpOnlyThats->end())
                    {
                        optionFound = true;
                        break;
                    }
                }

                if (!optionFound)
                    skipThisOption = true;
            }

            if (skipThisOption)
                continue;

            if (style==PrintHelpStyle::bash_complete)
            {
                for( const auto &optName : optInfo.optNames )
                {
                    if (optName.size()<=1)
                        oss<<" -";
                    else
                        oss<<" --";
                    oss<<optName;

                    ++totalNamesPrinted;
                }

                continue;
            }

            if (style==PrintHelpStyle::clink_complete)
            {
                for( const auto &optName : optInfo.optNames )
                {
                    if (totalNamesPrinted)
                        oss<<", ";
                    oss<<"\"";
                    if (optName.size()<=1)
                        oss<<"-";
                    else
                        oss<<"--";
                    oss<<optName;
                    oss<<"\"";

                    ++totalNamesPrinted;
                }

                continue;
            }


            //oss<<"\n";
            //bool bFirst = true;
            size_t i = 0, sz = optInfo.optNames.size();
            //for( const auto &optName : optInfo.optNames )
            for(; i!=sz; ++i)
            {
                const auto &optName = optInfo.optNames[i];

                if (i==0)
                {
                    oss<<"\n";
                }
                else
                {
                    if (style==PrintHelpStyle::wiki || style==PrintHelpStyle::md)
                        oss<<",\n";
                    else
                        oss<<"\n";
                }

                if (style==PrintHelpStyle::wiki)
                    oss<<"**%%";
                else if (style==PrintHelpStyle::md)
                    oss<<"**";

                if (optName.size()<=1)
                    oss<<"-";
                else
                    oss<<"--";
                oss<<optName;
                if (!optInfo.param.empty())
                {
                    if (optInfo.paramOptional)
                        oss<<"[="<<optInfo.param<<"]";
                    else
                        oss<<"="<<optInfo.param;
                }

                if (style==PrintHelpStyle::wiki)
                    oss<<"%%**";
                else if (style==PrintHelpStyle::md)
                    oss<<"**";

                //oss<<"\n";
            }

            std::stringstream descExtraOss;

            if (optInfo.optType==OptionType::optFlag)
            {
                descExtraOss << "Flag option, allowed values: " 
                             <<        getHelpQuotChar() << "+"       << getHelpQuotChar() 
                             << "/" << getHelpQuotChar() << "1"       << getHelpQuotChar() 
                             << "/" << getHelpQuotChar() << "Y(es)"   << getHelpQuotChar() 
                             << "/" << getHelpQuotChar() << "T(rue)"  << getHelpQuotChar() 
                             << " or "
                             << "/" << getHelpQuotChar() << "-"       << getHelpQuotChar() 
                             << "/" << getHelpQuotChar() << "0"       << getHelpQuotChar() 
                             << "/" << getHelpQuotChar() << "N(o)"    << getHelpQuotChar() 
                             << "/" << getHelpQuotChar() << "F(alse)" << getHelpQuotChar() 
                             ;
            }

            if (optInfo.hasMinMax)
            {
                descExtraOss<<"Min value: "<<getHelpQuotChar()<<optInfo.minVal<<getHelpQuotChar()<<", "
                            <<"max value: "<<getHelpQuotChar()<<optInfo.maxVal<<getHelpQuotChar();
            }

            if (optInfo.optType==OptionType::optEnum)
            {
                //std::string enumNames = optInfo.getAllEnumNames( ", ", " or ", " / ", "'", "'" );
    // std::string getAllEnumNames( const std::string &sep
    //                            , const std::string &sepLast
    //                            , const std::string &alter
    //                            , const std::string &before = std::string()
    //                            , const std::string &after = std::string()
    //                            ) const

                std::string enumNames = optInfo.getAllEnumNames( ", "    // sep
                                                               , " or "  // sepLast
                                                               , "/"     // alter
                                                               , getHelpQuotString() // "'"     // before (quot)
                                                               , getHelpQuotString() // "'"     // after (quot)
                                                               );
                if (!enumNames.empty())
                   descExtraOss<<"Allowed values: "<<enumNames;
                   //descExtraOss<<"Enumeration: "<<enumNames;
            }

/*
        if (!hasDefVal)
            return std::string();

        if (!paramOptional)
            return std::string();
*/

            std::string initVal = optInfo.getInitialValString(getHelpQuotChar());
            if (!initVal.empty())
            {
                if (descExtraOss.str().empty())
                {
                    descExtraOss<<"Initial value: ";
                }
                else
                {
                    descExtraOss<<", initial value: ";
                }
                descExtraOss<<initVal;
            }

            if (optInfo.hasDefVal && optInfo.paramOptional)
            {
                std::string defVal = optInfo.getDefValString(getHelpQuotChar());
                if (!defVal.empty())
                {
                    if (descExtraOss.str().empty())
                        descExtraOss<<"Default value: ";
                    else
                        descExtraOss<<", default value: ";

                    descExtraOss<<defVal;
                }
            }

            std::string descr = umba::text_utils::textAppendDot(optInfo.description);
            //if (!descr.empty() && descr.back()!='.')
            //    descr.append(1, '.');

            std::string descExtra = descExtraOss.str();
            if (!descExtra.empty())
                descExtra = std::string("\n") + descExtra;

            descr.append(descExtra);
            descr = umba::text_utils::textAppendDot(descr);

            if (style==PrintHelpStyle::wiki || style==PrintHelpStyle::md)
            {
                std::string tmp = descr;
                umba::string_plus::trim(tmp);
                if (!tmp.empty() && tmp.back()!='.')
                    tmp.append(1, '.');
                oss<<" - " <<tmp;
            }
            else
            {
                oss<<"\n";
                oss<< umba::text_utils::textAddIndent(umba::text_utils::formatTextParas( descr, width, umba::text_utils::TextAlignment::left ), "    " );
            }

            oss<<"\n";

        }


/*
enum class OptionType
{
    optUnknown     ,

    optFlag        ,
    optInt         ,
    optString      ,
    optEnum        ,

    optIntList     ,
    optStringList  ,
    optEnumList
};


struct CommandLineOptionInfo
{
    std::vector< std::string > optNames;
    bool                       internalOption = false;
    std::string                param;
    bool                       paramOptional  = false;
    std::string                description;

    OptionType                 optType;

    bool                       hasDefVal;
    bool                       hasMinMax;

    bool                       boolDefVal;
    int                        intDefVal;    // also enum defVal
    std::string                stringDefVal;

    int                        minVal;
    int                        maxVal;

    std::map<std::string, int> enumVals;
*/



        if (style==PrintHelpStyle::bash_complete || style==PrintHelpStyle::clink_complete)
        {
            oss<<suffix;
        }

        return oss.str();
    }

    virtual std::string makeText( std::string::size_type width, PrintHelpStyle printHelpStyle, const std::set<std::string> *pHelpOnlyThats ) override
    {
        return makeText( width, printHelpStyle, 0, 0, false, pHelpOnlyThats );
    }

    virtual std::string makeText( std::string::size_type width, PrintHelpStyle printHelpStyle, std::string *pPrefix, std::string *pSuffix, bool useExt ) override
    {
        return makeText( width, printHelpStyle, pPrefix, pSuffix, useExt, 0 );
    }

    //virtual std::string makeText( std::string::size_type width, PrintHelpStyle style, std::string *pPrefix, std::string *pSuffix, bool useExt, const std::set<std::string> *pHelpOnlyThats ) override



}; // class CommandLineOptionCollectorImplBase
// #include "umba/warnings/interface_end.h"



inline
bool checkRcScriptFileLineForUpdate( std::string line, const std::string & lineTextToCompare, const std::string & prefix, const std::string & suffix )
{
    umba::string_plus::trim(line);

    if (prefix.empty() && suffix.empty())
    {
        return line == lineTextToCompare;
    }
    else
    {
        //return startsWith( line, prefix ) && endsWith( line, suffix );
        return umba::string_plus::starts_with(line, prefix) && umba::string_plus::ends_with(line, suffix);
    }
}

inline
void updateRcScriptFileText( std::string &fileText, const std::string & updateLineText, const std::string & prefix, const std::string & suffix, bool simpleRemoveLine )
{
    std::string linefeed;
    std::vector< std::string > lines;
    umba::text_utils::textSplitToLines( fileText, lines, linefeed );

    std::vector< std::string > resLines;

    //size_t i = 0, sz = lines.size();

    bool actionPerformed = false;

    std::vector< std::string >::const_iterator lit = lines.begin();
    for(; lit != lines.end(); ++lit )
    {
        if (actionPerformed)
        {
            resLines.push_back(*lit);
            continue;
        }

        if ( !checkRcScriptFileLineForUpdate( *lit, updateLineText, prefix, suffix ) )
        {
            resLines.push_back(*lit);
            continue;
        }

        // found the line
        if (actionPerformed)
        {
           // action already performed? Hm-m-m...
           // Do nothing
            resLines.push_back(*lit);
            continue;
        }

        actionPerformed = true;

        if (simpleRemoveLine)
        {
            continue; // simply skip it
        }

        resLines.push_back(updateLineText);
    }


    if (!actionPerformed) // simply add line to the end of the file
        resLines.push_back(updateLineText);

    fileText.clear();

    for( const auto &l : resLines )
    {
        fileText.append(l);
        fileText.append(linefeed);
    }

}

inline
bool updateRcScriptFile( const std::string &fileName, const std::string & updateLineText, const std::string & prefix, const std::string & suffix, bool simpleRemoveLine )
{
    std::string fileText;

    readBinaryFile( fileName, fileText );

    // Пока ошибки нет
    //if (!readBinaryFile( fileName, fileText ))
    //    return false;

    updateRcScriptFileText( fileText, updateLineText, prefix, suffix, simpleRemoveLine );
    std::ofstream output( fileName, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );

    // А вот теперь - точно ошибка
    if (!output)
    {
        return false;
    }

    output.write( fileText.data(), (std::streamsize)fileText.size() );

    return true;
}


// .bashrc
//.bash_profile
// if [ -f ~/.bashrc ]; then . ~/.bashrc; fi

inline
bool updateAutocompletionBashRc( ICommandLineOptionCollector *pCol, bool simpleRemoveLine )
{
    std::string pref, suf, text = pCol->makeText( 0, PrintHelpStyle::bash_complete, &pref, &suf, false );
    if (!updateRcScriptFile(umba::filename::appendPath(umba::filesys::getCurrentUserHomeDirectory(), umba::string_plus::make_string<std::string>(".bashrc") ), text, pref, suf, simpleRemoveLine ))
        return false;
    #if defined(WIN32) || defined(_WIN32)
        text = pCol->makeText( 0, PrintHelpStyle::bash_complete, &pref, &suf, true );
        //if (!updateRcScriptFile( appendPath( getUserFolderPath(), ".bashrc" ), text, pref, suf, simpleRemoveLine ))
        if (!updateRcScriptFile(umba::filename::appendPath(umba::filesys::getCurrentUserHomeDirectory(), umba::string_plus::make_string<std::string>(".bashrc") ), text, pref, suf, simpleRemoveLine))

            return false;
    #endif

    return true;
}

inline
bool updateAutocompletionBashProfile( ICommandLineOptionCollector *pCol, bool simpleRemoveLine )
{
    UMBA_USED(pCol);
    //std::string pref, suf, text = pCol->makeText( 0, PrintHelpStyle::bash_complete, &pref, &suf );
    return updateRcScriptFile(umba::filename::appendPath(umba::filesys::getCurrentUserHomeDirectory(), umba::string_plus::make_string<std::string>(".bash_profile") ), "if [ -f ~/.bashrc ]; then . ~/.bashrc; fi", std::string(), std::string(), simpleRemoveLine );
}

inline
bool updateAutocompletionBashScripts( ICommandLineOptionCollector *pCol, bool simpleRemoveLine )
{
    return updateAutocompletionBashRc( pCol, simpleRemoveLine ) && updateAutocompletionBashProfile( pCol, simpleRemoveLine );
}

// D:\clink\clink.lua
inline
bool updateAutocompletionCLinkScripts( ICommandLineOptionCollector *pCol, bool simpleRemoveLine, std::string clinkPathFile )
{
    // CLink - чисто виндовая штука
    #if defined(WIN32) || defined(_WIN32)

    std::string pathName = "d:/clink/";
    std::string fileName = "clink.lua";

    if (clinkPathFile.size()==1)
    {
        if ( (clinkPathFile[0]>='a' && clinkPathFile[0]<='z')
          || (clinkPathFile[0]>='A' && clinkPathFile[0]<='Z')
           )
        {
            pathName[0] = clinkPathFile[0];
        }
        else
        {
            return false;
        }
    }
    else if (!clinkPathFile.empty())
    {
        pathName = clinkPathFile;
    }

    umba::filename::stripLastPathSep(pathName);

    // https://docs.microsoft.com/ru-ru/cpp/c-runtime-library/reference/stat-functions?view=vs-2019

    bool isFile = true;

    #if defined(WIN32) || defined(_WIN32)
        struct _stat s;
        if( _stat( pathName.c_str(), &s ) == 0 )
        {
            if ( s.st_mode & _S_IFDIR )
                isFile = false;
            else if (s.st_mode & _S_IFREG)
                isFile = true;
            else
                return false;
        }
    #else
        struct stat s;
        if( stat( pathName.c_str(), &s ) == 0 )
        {
            if ( s.st_mode & S_IFDIR )
                isFile = false;
            else if (s.st_mode & S_IFREG)
                isFile = true;
            else
                return false;
        }
    #endif
        else
        {
            // stat failed
            return false;
        }

    if (isFile)
    {
        clinkPathFile = pathName;
    }
    else
    {
        clinkPathFile = umba::filename::appendPath( pathName, fileName );
    }

    std::string pref, suf, text = pCol->makeText( 0, PrintHelpStyle::clink_complete, &pref, &suf, false );
    if (!updateRcScriptFile( clinkPathFile, text, pref, suf, simpleRemoveLine ))
        return false;

    #if defined(WIN32) || defined(_WIN32)
        text = pCol->makeText( 0, PrintHelpStyle::clink_complete, &pref, &suf, true );
        if (!updateRcScriptFile( clinkPathFile, text, pref, suf, simpleRemoveLine ))
            return false;
    #endif

    #endif // WIN32

    return true;


}

template< typename StreamGetter >
inline
int autocompletionInstaller ( ICommandLineOptionCollector *pCol, CommandLineOption &opt, PrintHelpStyle printHelpStyle, bool installMode, const StreamGetter &streamGetter )
{
    bool useBash  = false;
    bool useCLink = false;

    if (printHelpStyle == PrintHelpStyle::bash_complete)
        useBash = true;
    else if (printHelpStyle == PrintHelpStyle::clink_complete)
        useCLink = true;
    else if (printHelpStyle == PrintHelpStyle::normal)
    {
        useBash = true;
        useCLink = true;
    }
    else
    {
        //LOG_ERR_OPT
        streamGetter(true)<<"Invalid subkey; --wiki/--md used? ("<<opt.getOptionNames()<<")\n";
        return -1;
    }

    // #if defined(WIN32) || defined(_WIN32)
    // #else
    //     useCLink = false; // --clink simple ignored
    // #endif

    if (useBash)
    {
        if (!updateAutocompletionBashScripts(pCol, !installMode))
        {
            //LOG_ERR_OPT
            streamGetter(true)<<"Failed to update bash config ("<<opt.getOptionNames()<<")\n";
            return -1;
        }
    }

    #if defined(WIN32) || defined(_WIN32)
    if (useCLink)
    {
        if (!updateAutocompletionCLinkScripts(pCol, !installMode, opt.optArg))
        {
            //LOG_ERR_OPT
            streamGetter(true)<<"Failed to update clink config ("<<opt.getOptionNames()<<")\n";
            return -1;
        }
    }
    #endif

    streamGetter(false)<<"Autocompletion successfully installed\n";

    return 1;
}


inline
bool argsHasHelpOption( const std::vector<std::string> &args )
{
    for( const auto &a : args)
    {
        CommandLineOption opt(a);

        if (opt.isPureHelpOption())
            return true;
    }

    return false;
}


inline
bool argsFilterForHelpOption( std::vector<std::string> &args, std::set<std::string> &argsNeedHelp, const std::set<std::string> &argsAllowedForHelp )
{
    std::vector<std::string> argsTmp;
    argsTmp.reserve(args.size());

    for( const auto &a : args )
    {
        CommandLineOption opt(a);

        auto b = a;

        //ltrim(b, "-");
        umba::string_plus::ltrim(b, [](char ch) {return ch == '-'; } );

        bool allowedForHelp = argsAllowedForHelp.find(b)!=argsAllowedForHelp.end();

        //if (!allowedForHelp) // allowed to use with --help in special way
            argsTmp.push_back(a);

        if (opt.isPureHelpOption())
        {
            //args = argsTmp;
            swap(args, argsTmp);
            return true;
        }

        //ltrim(a, "-");
        if (!allowedForHelp)
            argsNeedHelp.insert(b); // print help only for that args
    }

    argsNeedHelp.clear();

    return false;
}

inline
bool argsFilterForHelpOption( std::vector<std::string> &args, std::set<std::string> &argsNeedHelp )
{
    return argsFilterForHelpOption( args, argsNeedHelp, CommandLineOption::getHelpStyleOptions() );
}


/*
template<typename StringType>
struct ArgParsingContext
{
    umba::program_location::ProgramLocation<StringType>  programLocationInfo;

    std::vector<StringType>   args;
    bool                      hasHelpOption   = false;
    bool                      disableBuiltins = false; // prevent to parse conf/$(AppExeName).options
    bool                      quet            = false;
    bool                      mustExit        = false; // prevent to parse
    std::set<StringType>      argsNeedHelp    ;

};
*/



/*

struct BuiltinOptionsLocationFlag
{
    const static unsigned     appGlobal     = 0x0001;
    const static unsigned     customGlobal  = 0x0002;
    const static unsigned     userLocal     = 0x0004;
    const static unsigned     all           = 0x0007;
};

*/


template<typename StringType, typename ArgParser, typename OptionsCollector >
struct ArgsParser
{
    UMBA_RULE_OF_FIVE(ArgsParser, default, default, default, default, default);
    //ArgParsingContext<StringType>         ctx;

    umba::program_location::ProgramLocation<StringType>  programLocationInfo;

    std::vector<StringType>   args;
    bool                      hasHelpOption          = false;
    // bool                      disableAppBuiltins     = false; //!< prevent to parse conf/$(AppExeName).options
    // bool                      disableCustomBuiltins  = false; //!< prevent to parse conf/$(AppExeName).user
    // bool                      disableUserBuiltins    = false; //!< prevent to parse $(Home)\.$(AppExeName).options OR used $(Home)\.$(AppExeName)\builtin.options

    unsigned                  builtinsOptions        = umba::program_location::BuiltinOptionsLocationFlag::all;

    bool                      quet                   = false;
    bool                      mustExit               = false; //!< prevent to continue parsing
    std::set<StringType>      argsNeedHelp           ;


    ArgParser                             argParser;
    OptionsCollector                      optionsCollector;
    std::stack<StringType>                optFiles;


    StringType getAppRoot() const
    {
        return programLocationInfo.getAppRoot();
    }

    StringType getAppConfPath() const
    {
        return programLocationInfo.getAppConfPath();
    }

    StringType getBasePath() const
    {
        StringType basePath;
        if (optFiles.empty())
            basePath = umba::filesys::nameConvert<StringType>(umba::filesys::getCurrentDirectory());
        else
            basePath = umba::filename::getPath(optFiles.top());
    
        return basePath;
    }
    
    StringType makeAbsPath( StringType p )
    {
        //return umba::filename::makeCanonical(umba::filename::makeAbsPath( p, getBasePath() ));
        return umba::filename::makeAbsPath( p, getBasePath() );
    }

    void pushOptionsFileName(const StringType &fname)
    {
        optFiles.push(fname);    
    }

    void popOptionsFileName()
    {
        if (!optFiles.empty())
            optFiles.pop();
    }


    std::string getBuiltinsOptFileName(unsigned flag) const { return programLocationInfo.getBuiltinOptionsFilename(flag); }

    template< typename StreamType >
    StreamType& printBuiltinFileNames( StreamType &s ) const
    {
        if (builtinsOptions)
            s << "\nBuiltin option files:\n";

        using namespace umba::program_location;

        if (builtinsOptions&BuiltinOptionsLocationFlag::appGlobal   )
            s << "  AppGlobal  : " << programLocationInfo.getBuiltinOptionsFilename( BuiltinOptionsLocationFlag::appGlobal ) << "\n";

        if (builtinsOptions&BuiltinOptionsLocationFlag::customGlobal)
            s << "  UserGlobal : " << programLocationInfo.getBuiltinOptionsFilename( BuiltinOptionsLocationFlag::customGlobal ) << "\n";

        if (builtinsOptions&BuiltinOptionsLocationFlag::userLocal   )
            s << "  UserLocal  : " << programLocationInfo.getBuiltinOptionsFilename( BuiltinOptionsLocationFlag::userLocal ) << "\n";

        // if (builtinsOptions)
        //     s << "\n";

        return s;
    }

    template< typename StreamType >
    StreamType& printHelpPage( StreamType &os, const std::string &usageString, std::string usageComment, const std::string &helpText ) const
    {
        bool mdMode = getPrintHelpStyle()==umba::command_line::PrintHelpStyle::md;

        umba::string_plus::trim(usageComment);
        if (!usageComment.empty() && usageComment.back()!='.')
            usageComment.append(1, '.');

        if (mdMode)
        {
            os << "---\n"
               << "Title: " << appFullName << " command line options\n"
               << "App-version: " << appVersion << "\n"
               << "---\n\n";
            os << "# Usage\n\n```\n";
        }
        else
        {
            os << "Usage: ";
        }

        os << programLocationInfo.exeName << " " << usageString << "\n";
           // << " [OPTIONS] input_file [output_file]\n"
           //  << "  If output_file not taken, STDOUT used\n";

        if (mdMode)
        {
            os  << "```\n\n";
        }

        if (!usageComment.empty())
        {

            os << usageComment << "\n\n";
        }

        if (mdMode)
        {
            os << "\n# Options\n\n";
        }
        else
        {
            os << "\nOptions:\n\n";
        }

        os << helpText << "\n";

        return os;
    }

    PrintHelpStyle getPrintHelpStyle() const
    {
        return optionsCollector.getPrintHelpStyle();
    }


    //int callArgParser( std::string a, bool fBuiltin, bool ignoreInfos )
    int callArgParser( StringType a, bool fBuiltin, bool ignoreInfos )
    {
        umba::string_plus::trim(a);

        if (a.empty())
        {
            // throw std::runtime_error("invalid (empty) argument");
            // return -1;

            return 0; // Просто игнорим
        }

        ICommandLineOptionCollector *pCol = &optionsCollector;

        umba::command_line::CommandLineOption opt(a, pCol);

        pCol->setCollectMode( opt.isHelpOption() );

        return argParser( a, opt, *this,  /* ctx,  */ pCol, fBuiltin, ignoreInfos );
    }


    bool parseOptionsFile( std::istream &optFileIfs )
    {
        std::vector<StringType> opts;
        umba::command_line::readOptionsFile(optFileIfs, opts);
        for( auto optLine : opts)
        {
            umba::string_plus::trim(optLine);
            if (optLine.empty())
                continue;

            if (umba::command_line::isComment( optLine ))
                continue;

            int paRes = callArgParser(optLine, true, true); // argParser( optLine, *this, &optionsCollector, true, true ); // bool fBuiltin, bool ignoreInfos
            if (paRes)
            {
                /* ctx. */ mustExit = true;
               return paRes<0 ? false : true;
            }
        }
    }

    //! Must return: 0 - ok, 1 normal stop, -1 - error
    // virtual int parseArg( std::string a, ICommandLineOptionsCollector *pCol, bool fBuiltin, bool ignoreInfos) = 0;

    //! Returns true if ok, false if some error occured
    bool parseOptionsFile( const StringType &optionsFileName )
    {
        // if (disableBuiltins)
        //     return true;

        // std::string optionsFileName = umba::filename::appendPath<std::string>(programLocationInfo.confPath, optFilename );

        // Переделываем, как разбор обычного респонз файла

        // int paRes = callArgParser(optLine, true, true); // argParser( optLine, *this, &optionsCollector, true, true ); // bool fBuiltin, bool ignoreInfos
        // if (paRes)
        // {
        //     /* ctx. */ mustExit = true;
        //    return paRes<0 ? false : true;
        // }

#if 1
        std::vector<StringType> opts;
        umba::command_line::readOptionsFile(optionsFileName, opts );
        for( auto optLine : opts)
        {
            umba::string_plus::trim(optLine);
            if (optLine.empty())
                continue;

            if (umba::command_line::isComment( optLine ))
                continue;

            int paRes = callArgParser(optLine, true, true); // argParser( optLine, *this, &optionsCollector, true, true ); // bool fBuiltin, bool ignoreInfos
            if (paRes)
            {
                /* ctx. */ mustExit = true;
               return paRes<0 ? false : true;
            }
        }
#endif
        return true;
    }

    //! Parses predefined files
    /*!
        $(AppRoot)/conf/$(AppExeName).options     - file from distribution, can be skipped
     */
    //! If userConfFileOnly==true, used $(Home)\.$(AppExeName).options, else used $(Home)\.$(AppExeName)\builtin.options
    bool parseStdBuiltins( bool userConfFileOnly = true )
    {
        // if (disableBuiltins)
        //     return true;
        UMBA_USED(userConfFileOnly);

        auto filenames = programLocationInfo.getBuiltinOptionsFilenames(builtinsOptions);

        auto it = filenames.begin();

        for(; it!=filenames.end(); ++it)
        {
            //std::cout << "Parse std builtin file: " << *it << "\n";
#if 1
            auto strResponseFile = StringType(1, typename StringType::value_type('@')) + *it;
            int paRes = callArgParser(strResponseFile, true, true); // argParser( optLine, *this, &optionsCollector, true, true ); // bool fBuiltin, bool ignoreInfos
            if (paRes)
            {
                /* ctx. */ mustExit = true;
               return paRes<0 ? false : true;
            }
#else

            if (!parseOptionsFile( *it ) )
                return false;

#endif

        }

        return true;
    }


    bool parse()
    {
        // std::vector<StringType>   args;
        for( const auto &a : args)
        {
            int paRes = callArgParser(a, false, false);
            //int paRes = argParser( a, opt, *this,  /* ctx,  */ pCol, false, false );
            if (paRes)
            {
               /* ctx. */ mustExit = true;
               return paRes<0 ? false : true;
            }
        }

        return true;
    }


}; // struct ArgsParser




template<typename StringType, typename ArgParser, typename OptionsCollector> inline
ArgsParser<StringType,ArgParser,OptionsCollector>
makeArgsParserImpl( const ArgParser                                            &argParser
                  , const OptionsCollector                                     &optionsCollector
                  , int                                                        argc
                  , typename StringType::value_type                            **argv
                  , const umba::program_location::ProgramLocation<StringType>  &programLocationInfo
                  )
{
    ArgsParser<StringType,ArgParser,OptionsCollector>  argsParser;// ctx;

    argsParser.argParser               = argParser;
    argsParser.optionsCollector        = optionsCollector;
    argsParser.programLocationInfo = programLocationInfo;

    if (argsParser.programLocationInfo.exeFullName.empty())
       argsParser.programLocationInfo = umba::program_location::getProgramLocation(argc, argv); //!!! Надо ProgramLocation проверить на юникод

    argsParser.args = prepareArgs( argc, argv );

    argsParser.hasHelpOption = argsFilterForHelpOption(argsParser.args, argsParser.argsNeedHelp );


    for( const auto & a : argsParser.args)
    {
        CommandLineOption opt(a);

        if (!opt.isOption())
            continue;

        if (opt.isBuiltinsDisableOptionMain())
            argsParser.builtinsOptions &= ~umba::program_location::BuiltinOptionsLocationFlag::appGlobal   ;

        if (opt.isBuiltinsDisableOptionCustom())
            argsParser.builtinsOptions &= ~umba::program_location::BuiltinOptionsLocationFlag::customGlobal;

        if (opt.isBuiltinsDisableOptionUser())
            argsParser.builtinsOptions &= ~umba::program_location::BuiltinOptionsLocationFlag::userLocal   ;


        // else if (opt.isOption("no-builtin-includes"))
        //     disableBuiltinIncludes = true;
        else if (opt.isSomeKindOfQuet() && !argsParser.hasHelpOption)
        {
            argsParser.quet = true;
        }
        else if (opt.isWhereOption())
        {
            if constexpr (sizeof(typename StringType::value_type)>sizeof(char))
                std::wcout << programLocationInfo.exeFullName << L"\n";
            else
                std::cout << programLocationInfo.exeFullName << "\n";

            argsParser.mustExit = true;
        }

        if (argsParser.mustExit)
            break;
    }

    return argsParser;
}


template<typename ArgParser, typename OptionsCollector> inline
ArgsParser<std::string,ArgParser,OptionsCollector>
makeArgsParser( const ArgParser         &argParser
              , const OptionsCollector  &optionsCollector
              , int                     argc
              , char                    **argv
              , const umba::program_location::ProgramLocation<std::string> &programLocationInfo = umba::program_location::ProgramLocation<std::string>()
                            //&programLocationInfo = umba::program_location::getProgramLocation(argc, argv)
              )
{
    return makeArgsParserImpl<std::string, ArgParser, OptionsCollector>( argParser, optionsCollector, argc, argv, programLocationInfo );
}

template<typename ArgParser, typename OptionsCollector> inline
ArgsParser<std::wstring,ArgParser,OptionsCollector>
makeArgsParser( const ArgParser        &argParser
              , const OptionsCollector &optionsCollector
              , int                    argc
              , wchar_t                **argv
              , const umba::program_location::ProgramLocation<std::wstring> &programLocationInfo = umba::program_location::ProgramLocation<std::wstring>()
                            //&programLocationInfo = umba::program_location::getProgramLocation(argc, argv)
              )
{
    return makeArgsParserImpl<std::wstring, ArgParser, OptionsCollector>( argParser, optionsCollector, argc, argv, programLocationInfo );
}

/*
template<typename StringType> inline
ProgramLocation<StringType> getProgramLocationImpl( const StringType &argv0
                                                  , StringType        overrideExeName = StringType() //!< Used to make single user conf name for multiple exe's
                                                  , const StringType &confFolderName  = umba::string_plus::make_string<StringType>(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME)
                                                  )


inline
ProgramLocation<std::string> getProgramLocation( int argc, char **argv
                                               , std::string overrideExeName       = std::string()
                                               , const std::string &confFolderName = umba::string_plus::make_string<std::string>(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME)
                                               )

inline
ProgramLocation<std::wstring> getProgramLocation( int argc, wchar_t **argv
                                                , std::wstring overrideExeName       = std::wstring()
                                                , const std::wstring &confFolderName = umba::string_plus::make_string<std::wstring>(UMBA_PROGRAM_LOCATION_DEF_CONF_FOLDER_NAME)
                                                )

*/


/*

Для обработки команд, по примеру git'а: 
  git remote add
  git stash push
  git stash pop
  git stash list
  git stash clear
  git submodule add
  git submodule update
  git submodule foreach git pull
  git worktree list
Какие-то опции могут работать только с определёнными подкомандами
Какие-то опции могут работать глобально

После того, как команда с подкомандами накоплены, могут идти опции команды, а также
какие-то дополнительные аргументы. Это могут быть как файлы, так и какие-то имена, 
не связанные с файловой системой.

По умолчанию - это файлы, и на его базе делается абсолютное имя файла.
Но как быть, если это не имя файла?
Добавим обработчик трансформации параметра

*/


// Переделать в шаблон, StringType вместо std::string?
struct CommandInfo
{
    // int handler(const std::string &fullCommandStr, const std::vector<std::string> &inputList)
    using CommandHandlerType = std::function<int(const std::string &, const std::vector<std::string> &)>;
    using command_handler_t  = CommandHandlerType;

    // std::string handler( const std::string &fullCommandStr
    //                    , const std::vector<std::string> &inputList // previosly added params
    //                    , const std::string &paramValue
    //                    , const std::string &cwd
    //                    )
    using ParameterTransformHandlerType = std::function<std::string( const std::string &
                                                                   , const std::vector<std::string> &
                                                                   , const std::string &
                                                                   , const std::string &
                                                                   )
                                                       >;
    using parameter_transform_handler_t = ParameterTransformHandlerType;



    std::string                          commandName;
    std::string                          usageInfo;
    std::string                          brief;
    std::string                          description;
    std::map<std::string, CommandInfo>   subCommands;
    std::unordered_set<std::string>      allowedOptions;

    CommandHandlerType                   commandHandler;  // по умолчанию в корневом элементе нет никакого обработчика, и пустая команда недопустима
    ParameterTransformHandlerType        parameterTransformHandler;
    // std::string                          helpString;
    std::size_t                          maxInputParams = std::size_t(-1);
    bool                                 rawMode = false;


    //--------------------------------------------------
    static
    std::string findLongestName(const std::vector<std::string> &names)
    {
        if (names.empty())
            return std::string();

        std::string res = names.front();

        for(const auto &n : names)
        {
            if (n.size() > res.size())
                res = n;
        }

        return res;
    }

    //--------------------------------------------------
    static
    std::vector<std::string> splitHelper(const std::string &str, char ch)
    {
        auto tmpVec = umba::string_plus::split(str, ch);
        std::vector<std::string> resVec; resVec.reserve(tmpVec.size());

        for(auto tmpStr : tmpVec)
        {
            umba::string_plus::trim(tmpStr);
            if (!tmpStr.empty())
                 resVec.push_back(tmpStr);
        }

        return resVec;
    }

    //--------------------------------------------------
    static
    std::vector<std::string> splitCommandStr(const std::string &str)
    {
        return splitHelper(str, ' ');
    }

    //--------------------------------------------------
    static
    std::vector<std::string> splitOptionsStr(const std::string &str)
    {
        return splitHelper(str, ',');
    }

    //--------------------------------------------------



    //--------------------------------------------------
    // Добавляет команду (если её ещё нет) и устанавливает дефолтный обработчик, если он ещё не задан
    template<typename CmdIter>
    CommandInfo* addCommandImpl(CmdIter b, CmdIter e)
    {
        if (b==e)
        {
            if (!commandHandler)
            {
                commandHandler = [](const std::string &fullCmd, const std::vector<std::string> &inputs) -> int
                                 {
                                     // throw std::runtime_error("command handler for command sequence '" + fullCmd + "' not implemented");
                                     throw std::runtime_error("command handler for command '" + fullCmd + "' not implemented "
                                                              "(parameters: '" + umba::shellapi::makeSystemFunctionCommandString(inputs) + "')"
                                                             );
                                 };
            }

            return this;
        }
        else
        {
            auto &subCommandInfo = subCommands[*b];
            subCommandInfo.commandName = *b;
            return subCommandInfo.addCommandImpl(std::next(b), e);
        }
    }

    //--------------------------------------------------
    CommandInfo& addCommand(const std::vector<std::string> &cmdSeq)
    {
        CommandInfo* pCommandInfo = addCommandImpl(cmdSeq.begin(), cmdSeq.end());
        if (!pCommandInfo)
        {
            throw std::runtime_error("umba::command_line::CommandInfo::addCommand: something goes wrong");
        }

        return *pCommandInfo;
    }

    //--------------------------------------------------
    CommandInfo& addCommand(const std::string &cmdSequenceStr) // space separated
    {
        auto cmdSeq = splitCommandStr(cmdSequenceStr);
        return addCommand(cmdSeq);
    }

    //--------------------------------------------------



    //--------------------------------------------------
    // Хэндлер вызывается N+1 раз - первый раз для корневого элемента без имени
    template<typename TraverseHandler> // bool traverseHandler(const CommandInfo* pCommandInfo)
    const CommandInfo* traverseCommandSequenseImpl(const std::vector<std::string> &cmdSequence, TraverseHandler traverseHandler, std::string *pCommandFullName=0) const
    {
        std::string fullName;

        const CommandInfo* pCommandInfo = this;
        if (!traverseHandler(pCommandInfo))
             return pCommandInfo;

        for(auto it= cmdSequence.begin(); it!= cmdSequence.end(); ++it)
        {
            if (fullName.empty())
            {
                fullName = *it;
            }
            else
            {
                if (!it->empty())
                {
                    fullName.append(1, ' ');
                    fullName.append(*it);
                }
            }

            auto subIt = pCommandInfo->subCommands.find(*it);
            if (subIt==pCommandInfo->subCommands.end())
            {
                if (pCommandFullName)
                   *pCommandFullName = fullName;
                return 0; // подкоманда не найдена (но имя не найденной команды всё равно возвращаем)
            }

            pCommandInfo = &subIt->second;
            if (!traverseHandler(pCommandInfo)) // хэндлер указал остановится
            {
                if (pCommandFullName)
                   *pCommandFullName = fullName;
                 return pCommandInfo;
            }
        }

        if (pCommandFullName)
           *pCommandFullName = fullName;

        return pCommandInfo; // Последовательность имён закончилась, выдаём указатель на инфу по последнему имени
    }

    //--------------------------------------------------
    template<typename TraverseHandler> // bool traverseHandler(const CommandInfo* pCommandInfo)
    const CommandInfo* traverseCommandSequense(const std::vector<std::string> &cmdSequence, TraverseHandler traverseHandler, std::string *pCommandFullName=0) const
    {
        return traverseCommandSequenseImpl(cmdSequence, traverseHandler, pCommandFullName);
    }

    //--------------------------------------------------
    template<typename TraverseHandler> // bool traverseHandler(const CommandInfo* pCommandInfo)
    CommandInfo* traverseCommandSequense(const std::vector<std::string> &cmdSequence, TraverseHandler traverseHandler, std::string *pCommandFullName=0)
    {
        return const_cast<CommandInfo*>(traverseCommandSequenseImpl(cmdSequence, traverseHandler, pCommandFullName));
    }

    //--------------------------------------------------



    //--------------------------------------------------
    const CommandInfo& findCommand(const std::vector<std::string> &cmdSequence, std::string *pCommandFullName=0) const
    {
        auto pCommandInfo = traverseCommandSequense(cmdSequence, [](const CommandInfo*) { return true; }, pCommandFullName );
        if (!pCommandInfo)
        {
             auto cmdStr = string::merge<std::string>(cmdSequence.begin(), cmdSequence.end(), ' ');
             throw std::runtime_error("umba::command_line::CommandInfo::findCommand: command '" + cmdStr + "' not found");
        }

        return *pCommandInfo;
    }

    //--------------------------------------------------
    CommandInfo& findCommand(const std::vector<std::string> &cmdSequence, std::string *pCommandFullName=0)
    {
        auto pCommandInfo = traverseCommandSequense(cmdSequence, [](const CommandInfo*) { return true; }, pCommandFullName );
        if (!pCommandInfo)
        {
             auto cmdStr = string::merge<std::string>(cmdSequence.begin(), cmdSequence.end(), ' ');
             throw std::runtime_error("umba::command_line::CommandInfo::findCommand: command '" + cmdStr + "' not found");
        }

        return *pCommandInfo;
    }

    //--------------------------------------------------
    const CommandInfo& findCommand(const std::string &cmdStr, std::string *pCommandFullName=0) const
    {
        auto cmdSeq = splitCommandStr(cmdStr);
        return findCommand(cmdSeq, pCommandFullName);
    }

    //--------------------------------------------------
    CommandInfo& findCommand(const std::string &cmdStr, std::string *pCommandFullName=0)
    {
        auto cmdSeq = splitCommandStr(cmdStr);
        return findCommand(cmdSeq, pCommandFullName);
    }

    //--------------------------------------------------

    

    //--------------------------------------------------
    template<typename TraverseHandler>
    bool traverseFinalCommands(TraverseHandler traverseHandler) // возвращает, надо ли продоложать
    {
        if (subCommands.empty())
        {
            return traverseHandler(this);
        }
        else
        {
            for(auto &[k, v] : subCommands)
            {
                if (!v.traverseFinalCommands(traverseHandler))
                    return false;
            }

            return true;
        }
    }

    //--------------------------------------------------



    //--------------------------------------------------
    CommandInfo& setOptions(const std::vector<std::string> &options)                      { allowedOptions.insert(options.begin(), options.end()); return *this; }

    CommandInfo& setOptionsMultiOpt(const std::string &optionsStr) /* comma separated */  { auto optVec = splitOptionsStr(optionsStr); return setOptions(optVec); }
    CommandInfo& setOptionsMultiOpt(const char* optionsStr) /* comma separated */         { auto optVec = splitOptionsStr(optionsStr); return setOptions(optVec); }
    CommandInfo& setOptionsMultiOpt(const std::vector<std::string> &options)              { allowedOptions.insert(options.begin(), options.end()); return *this; }

    // CommandInfo& setOptionSingleImpl(const std::string &optionStr)                        { allowedOptions.insert(optionStr); return *this; }
    // CommandInfo& setOptionSingleImpl(const char* optionStr)                               { allowedOptions.insert(std::string(optionStr)); return *this; }

    CommandInfo& setOptions(const std::string &optionsStr) /* comma separated */          { return setOptionsMultiOpt(optionsStr); }
    CommandInfo& setOptions(const char *optionsStr) /* comma separated */                 { return setOptionsMultiOpt(std::string(optionsStr)); }

    //--------------------------------------------------
    CommandInfo& setOptions( std::initializer_list<std::string> options) // list of comma separated strings
    {
        for(const auto &optcs: options)
            setOptions(optcs);
        return *this;
    }

    //--------------------------------------------------
    template<typename... Args>
    CommandInfo& setOptions(Args&&... args)
    {
        // Fold expression (C++17) – вызывает setOptions для каждого аргумента
        (setOptionsMultiOpt(std::forward<Args>(args)), ...);
        return *this;
    }
    //--------------------------------------------------



    //--------------------------------------------------
    CommandInfo& setCommandHandler(CommandHandlerType commandHandler_)
    {
        commandHandler = commandHandler_;
        return *this;
    }

    //--------------------------------------------------
    CommandInfo& setParameterTransformHandler(ParameterTransformHandlerType parameterTransformHandler_)
    {
        parameterTransformHandler = parameterTransformHandler_;
        return *this;
    }

    // //--------------------------------------------------
    // CommandInfo& setHelpString(const std::string &helpString_)
    // {
    //     helpString = helpString_;
    //     return *this;
    // }

    //--------------------------------------------------
    CommandInfo& setMaxInputParams(std::size_t maxInputParams_)
    {
        maxInputParams = maxInputParams_;
        return *this;
    }

    //--------------------------------------------------
    std::size_t getMaxInputParams() const
    {
        return maxInputParams;
    }

    //--------------------------------------------------
    CommandInfo& setRawMode(bool rawMode_)
    {
        rawMode = rawMode_;
        return *this;
    }

    //--------------------------------------------------
    bool getRawMode() const
    {
        return rawMode;
    }

    //--------------------------------------------------

    

    //--------------------------------------------------
    CommandInfo& setUsageInfo(const std::string& usageInfo_)
    {
        usageInfo = usageInfo_;
        return *this;
    }

    //--------------------------------------------------
    const std::string& getUsageInfo() const
    {
        return usageInfo;
    }

    //--------------------------------------------------
    CommandInfo& setBrief(const std::string& brief_)
    {
        brief = brief_;
        return *this;
    }

    //--------------------------------------------------
    const std::string& getBrief() const
    {
        return brief;
    }

    //--------------------------------------------------
    CommandInfo& setDescription(const std::string& description_)
    {
        description = description_;
        return *this;
    }

    //--------------------------------------------------
    const std::string& getDescription() const
    {
        return description;
    }

    //--------------------------------------------------



    //--------------------------------------------------
    // Полностью определённая подкоманда?
    bool isCompleteCommandSequence(const std::vector<std::string> &cmdSequence) const
    {
        const CommandInfo* pCommandInfo = traverseCommandSequense(cmdSequence, [](auto){ return true; });
        if (!pCommandInfo)
            return false; // подкоманда не найдена, считаем, что последовательность незакончена

        return pCommandInfo->subCommands.empty();
    }

    //--------------------------------------------------
    // Проверяет, можно ли ещё принимать подкоманды
    bool canReceiveSubCommands(const std::vector<std::string> &cmdSequence) const
    {
        const CommandInfo* pCommandInfo = traverseCommandSequense(cmdSequence, [](auto){ return true; });
        if (!pCommandInfo)
            return false; // текущая подкоманда не найдена, добавлять нельзя

        return !pCommandInfo->subCommands.empty();
    }

    //--------------------------------------------------
    bool isSubCommandAllowed(const std::vector<std::string> &curCmdSequence, const std::string &str) const
    {
        const CommandInfo* pCommandInfo = traverseCommandSequense(curCmdSequence, [](auto){ return true; });
        if (!pCommandInfo)
            return false; // родительская подкоманда не найдена, добавить нельзя вообще ничего

        auto subIt = pCommandInfo->subCommands.find(str);
        if (subIt==pCommandInfo->subCommands.end())
            return false;

        return true;
    }

    //--------------------------------------------------

    
    
    //--------------------------------------------------
    bool isOptionAllowed( const std::vector<std::string> &cmdSequence
                        , const std::vector<std::string> &optNames
                        ) const
    {
        bool bAllowed = false;

        traverseCommandSequense( cmdSequence
                               , [&](auto pCommandInfo) -> bool
                                 {
                                     for(const auto opt: optNames)
                                     {
                                         if (pCommandInfo->allowedOptions.find(opt)!=pCommandInfo->allowedOptions.end())
                                         {
                                             bAllowed = true;
                                             break;
                                         }
                                     }

                                     return !bAllowed;
                                 }
                               );
        return bAllowed;
    }

    //--------------------------------------------------
    bool isOptionAllowed( const std::vector<std::string> &cmdSequence
                        , const CommandLineOption &opt
                        ) const
    {
        return isOptionAllowed(cmdSequence, opt.getOptionNamesVector());
    }

    //--------------------------------------------------



    //--------------------------------------------------
    int executeCommand(const std::vector<std::string> &cmdSequence, const std::vector<std::string> &inputList) const
    {
        std::string fullName;

        try
        {
            const CommandInfo& ci = findCommand(cmdSequence, &fullName);

            if (fullName.empty())
            {
                if (!ci.commandHandler) // Для текущей команды (корневой) не задан обработчик - значит, пуcтую команду нельзя выполнить
                    throw std::runtime_error("umba::command_line::CommandInfo::execCommand: try to execute empty command");
    
                // Но обработчик может быть задан, и для пустой команды допустимо выполнение
                return ci.commandHandler(std::string(), inputList);
            }

            if (!ci.commandHandler)
            {
                throw std::runtime_error("umba::command_line::CommandInfo::execCommand: command '" + fullName + "' found, but command handler is not set");
            }
    
            return ci.commandHandler(fullName, inputList);

        }
        catch(const std::exception&)
        {
            // throw std::runtime_error("umba::command_line::CommandInfo::execCommand: command '" + fullName + "' not found at all");
            throw;
        }

    }

    //--------------------------------------------------

    
    
    //--------------------------------------------------
    std::string transformCommandParameter( const std::vector<std::string> &cmdSequence
                                         , const std::vector<std::string> &inputList
                                         , const std::string &paramValue
                                         , const std::string &cwd
                                         ) const
    {
        std::string fullName;

        try
        {
            const CommandInfo& ci = findCommand(cmdSequence, &fullName);

            if (ci.parameterTransformHandler)
            {
                return ci.parameterTransformHandler(fullName, inputList, paramValue, cwd);
            }
    
            // Трансформатора нет, поэтому считаем, что у нас имя файла

            if (paramValue.size()>2 && paramValue[0]=='$' && paramValue[0]=='(')
                 return paramValue; // Пути, начинающиеся с "$(" - не трансформируем - это макрос

            return umba::filename::makeAbsPath( paramValue, cwd );
        }

        catch(const std::exception&)
        {
            // throw std::runtime_error("umba::command_line::CommandInfo::execCommand: command '" + fullName + "' not found at all");
            throw;
        }

    }

    //--------------------------------------------------

}; // struct CommandInfo

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class CommandSequenceController
{

    CommandInfo                   commandInfo; // root command info
    mutable bool                  bSealed = false; // Запечатывание происходит, когда после команды или нескольких приходит опция
    std::vector<std::string>      commandSequence;
    std::vector<std::string>      inputList;


public:


    //--------------------------------------------------
    const std::vector<std::string>& getInputList() const { return inputList; }

    //--------------------------------------------------

    
    
    //--------------------------------------------------
    // Сырое добавление, без транформации
    void addInput(const std::string &inputParam)
    {
        std::size_t maxParams = findCommand(commandSequence).getMaxInputParams(); // std::size_t(-1);

        if (inputList.size() >= maxParams)
        {
            throw std::runtime_error("too many parameters for command: '" + string::merge<std::string>(commandSequence.begin(), commandSequence.end(), ' ') + "'");
        }

        inputList.push_back(inputParam);
    }

    //--------------------------------------------------
    // добавление с транформацией
    void addInput(const std::string &inputParam, const std::string &cwd)
    {
        if (isRawMode())
        {
            addInput(inputParam);
        }
        else
        {
            auto transformedParam = commandInfo.transformCommandParameter(commandSequence, inputList, inputParam, cwd);
            addInput(transformedParam);
        }
    }

    //--------------------------------------------------

    
    
    //--------------------------------------------------
    std::string getFullCommandStr() const
    {
        return string::merge<std::string>(commandSequence.begin(), commandSequence.end(), ' ');
    }

    std::string getFullCommandStr(const std::string &addCmd) const
    {
        auto cmdVec = commandSequence;
        cmdVec.push_back(addCmd);
        return string::merge<std::string>(cmdVec.begin(), cmdVec.end(), ' ');
    }

    //--------------------------------------------------

    
    
    //--------------------------------------------------
    void seal()           { bSealed = true; }
    bool isSealed() const { return bSealed; }

    bool isRawMode() const
    {
        if (!commandInfo.isCompleteCommandSequence(commandSequence))
            return false;

        return findCommand().getRawMode();
    }

    bool empty() const // true когда нет накопленных команд
    {
        return commandSequence.empty();
    }

    //--------------------------------------------------



    //--------------------------------------------------
    CommandInfo& addCommand(const std::string &cmdSequenceStr)                         { return commandInfo.addCommand(cmdSequenceStr); }

    const CommandInfo& findCommand(const std::vector<std::string> &cmdSequence, std::string *pCommandFullName=0) const  { return commandInfo.findCommand(cmdSequence, pCommandFullName); }
    CommandInfo&       findCommand(const std::vector<std::string> &cmdSequence, std::string *pCommandFullName=0)        { return commandInfo.findCommand(cmdSequence, pCommandFullName); }

    const CommandInfo& findCommand(const std::string &cmdSequenceStr, std::string *pCommandFullName=0) const            { return commandInfo.findCommand(cmdSequenceStr, pCommandFullName); }
    CommandInfo&       findCommand(const std::string &cmdSequenceStr, std::string *pCommandFullName=0)                  { return commandInfo.findCommand(cmdSequenceStr, pCommandFullName); }

    const CommandInfo& findCommand(std::string *pCommandFullName=0) const                                               { return commandInfo.findCommand(commandSequence, pCommandFullName); }
    CommandInfo&       findCommand(std::string *pCommandFullName=0)                                                     { return commandInfo.findCommand(commandSequence, pCommandFullName); }

    const CommandInfo& rootCommand() const { return commandInfo; }
    CommandInfo&       rootCommand()       { return commandInfo; }

    //--------------------------------------------------



    //--------------------------------------------------
    template<typename... Args>
    void addGlobalOptions(Args&&... args)
    {
        // Fold expression (C++17) – вызывает commandInfo.setOptions для каждого аргумента
        (commandInfo.setOptions(std::forward<Args>(args)), ...);
    }


    void addOptionsToFinalCommands(const std::string &optionsStr)
    {
        commandInfo.traverseFinalCommands( [&](CommandInfo *pCommandInfo)
                                           {
                                               pCommandInfo->setOptions(optionsStr);
                                               return true;
                                           }
                                         );
    }

    // void addOptionsToFinalCommands(std::initializer_list<std::string> options)
    // {
    //     commandInfo.traverseFinalCommands( [&](CommandInfo *pCommandInfo)
    //                                        {
    //                                            pCommandInfo->setOptions(options);
    //                                            return true;
    //                                        }
    //                                      );
    // }

    //--------------------------------------------------



    //--------------------------------------------------
    // Теоретически, можем запускать и не только не команды, которые насобирали
    int executeCommand(const std::vector<std::string> &cmdSeq, const std::vector<std::string> &inputs) const
    {
        return commandInfo.executeCommand(cmdSeq, inputs);
    }

    // Выполняет введённую команду (находит и запускает соотв функтор)
    int executeCommand() const
    {
        return executeCommand(commandSequence, inputList);
    }

    //--------------------------------------------------



    //--------------------------------------------------
    // Также "запечатывает" команду
    bool isOptionAllowed(const CommandLineOption &opt) const
    {
        // Проверяем опцию, если последовательность команд не пуста, то "запечатываем" - нельзя последовательность команд давать вперемешку с опциями
        if (!commandSequence.empty())
        {
            bSealed = true;
        }

        return commandInfo.isOptionAllowed(commandSequence, opt);
    }

    //--------------------------------------------------
    // Проверяет допустимость опции для текущей команды, а также "запечатывает" введённые команды
    bool isOptionAllowed(const CommandLineOption &opt, std::string &errMsg) const
    {
        if (isOptionAllowed(opt))
            return true;

        auto optNames = opt.getOptionNamesVector();
        auto optName = CommandInfo::findLongestName(optNames);
        if (optName.size()>1)
            optName = "--" + optName;
        else
            optName = "-" + optName;

        auto fullCmd = getFullCommandStr();
         
        if (fullCmd.empty())
            errMsg = "option '" + optName + "' is not a global option. It is a command specific option.";
        else
            errMsg = "option '" + optName + "' cannot be applied for command '" + getFullCommandStr() + "'.";

        return false;
    }

    //--------------------------------------------------

    
    
    //--------------------------------------------------
    // просто проверяет, достигли ли мы конца допустимой последовательности комманд, или нет. Не производит никакой проверки добавляемой команды на допустимость добавления именно её
    bool canAddSubCommand() const
    {
        if (bSealed)
            return false;

        if (!commandInfo.canReceiveSubCommands(commandSequence))
            return false;

        // тут добавим проверки по вкусу

        return true;
    }

    //--------------------------------------------------
    // производит проверку на допустимость добавления конкретной подкоманды
    bool isSubCommandAllowed(const std::string& cmd) const
    {
        // if (bSealed)
        //     return false;

        if (!canAddSubCommand())
            return false;

        return commandInfo.isSubCommandAllowed(commandSequence, cmd);
    }

    //--------------------------------------------------
    // производит проверку на допустимость добавления конкретной подкоманды, и при ошибке формирует сообщение
    bool isSubCommandAllowed(const std::string& cmd, std::string &errMsg) const
    {
        if (isSealed())
        {
            errMsg = "command and it's subcommands cannot be mixed with options.";
            return false;
        }

        if (!canAddSubCommand())
        {
            errMsg = "command and it's subcommands have already been completely entered.";
            return false;
        }    

        if (isSubCommandAllowed(cmd))
            return true;

        // auto fullCmd = getFullCommandStr();
        //  
        // if (fullCmd.empty())
        // {
        //     errMsg = "unknown command '" + fullCmd + "'";
        // }
        // else
        // {
        //     errMsg = "incompatible subcommand '" + cmd + "' for command '" + fullCmd + "'";
        // }

        auto fullWrongCmd = getFullCommandStr(cmd);

        errMsg = "unknown command '" + fullWrongCmd + "'.";

        return false;
    }

    //--------------------------------------------------
    // добавляет подкоманду, и при ошибке может выкидывать исключение
    bool appendSubCommandSequence(const std::string& cmd, bool throwError=true)
    {
        std::string errMsg;
        if (!isSubCommandAllowed(cmd, errMsg))
        {
            if (throwError)
                throw std::runtime_error(errMsg);
            return false;
        }

        commandSequence.push_back(cmd);

        return true;
    }

    //--------------------------------------------------
    // добавляет подкоманду, при ошибке сохраняе её текст в errMsg
    bool appendSubCommandSequence(const std::string& cmd, std::string &errMsg)
    {
        if (!isSubCommandAllowed(cmd, errMsg))
        {
            return false;
        }

        commandSequence.push_back(cmd);

        return true;
    }

    //--------------------------------------------------
    void addStandardCommonGlobalOptions()
    {
        addGlobalOptions("help,quet,verbose,version,color,build-info,build-info-x,no-builtin-options,no-custom-builtin-options,no-user-builtin-options,where");
    }

    //--------------------------------------------------
    void addStandardAutocompleteGlobalOptions()
    {
        addGlobalOptions("autocomplete-install,autocomplete-uninstall");
    }

    //--------------------------------------------------
    void addStandardHelpGlobalOptions()
    {
        addGlobalOptions("wiki,md,bash,clink");
    }

    //--------------------------------------------------
    void addAllStandardGlobalOptions()
    {
        addStandardCommonGlobalOptions();
        addStandardAutocompleteGlobalOptions();
        addStandardHelpGlobalOptions();
    }

    //--------------------------------------------------


    /*
        Для каждой команды должны быть установлены usageInfo и description.

        Для корневого элемента:
        - Если у корневого элемента нет подкоманд, то отсутствие хотя бы brief (или description?) - фатальная ошибка.
        - Если есть подкоманды, то корневой не выводится.

        Выводим инфу так (для многокомандного режима):
          command subcommand - DESCRIPTION
          Usage: main-exe command subcommand USAGE_INFO
          Available options:
            -opt1
            -opt2

        Или вот так:

          EXE [GLOBAL_OPTIONS...] COMMAND [SUBCOMMAND...] [COMMAND_OPTIONS...] [PARAMETERS...]  - главная строка, прошита (или берём из корня, если там задано)

          Commands:

            command1 subcommand1 - DESCRIPTION1

            command2 subcommand2 - DESCRIPTION2

            command3 subcommand3 - DESCRIPTION3

          Global Options:

            -opt1
            -opt2

          main-exe command subcommand USAGE_INFO
            DESCRIPTION DESCRIPTION DESCRIPTION DESCRIPTION DESCRIPTION DESCRIPTION

        Итого, варианты:
        - Инфа по всем подкомандам, при этом, два подварианта:
          - подкоманд нет
          - подкоманды есть
        - Инфа по команде, при этом варианты:
          - это конечная команда
          - есть подкоманды

        Разницы нет, только надо различать случаи, когда
        - подкоманд нет и это у нас корневой узел
        - подкоманд нет, но это не корень
          

    std::string                          commandName;
    std::string                          usageInfo;
    std::string                          brief;
    std::string                          description;
    std::map<std::string, CommandInfo>   subCommands;
    std::unordered_set<std::string>      allowedOptions;

    // pCol->makeText( 78, &argsParser.argsNeedHelp ) - генерит текст справки по опциям, вроде во всех форматах (normal/md/wiki)

    // pCol->getPrintHelpStyle() - возвращает стиль форматирования
    // umba::::textAddIndent(umba::text_utils::formatTextParas( descr, width, umba::text_utils::TextAlignment::left ), "    " );
    // if (style==PrintHelpStyle::wiki || style==PrintHelpStyle::md)
    // umba::command_line::ICommandLineOptionCollector *pCol
    // std::set<StringType>      argsNeedHelp

    argsParser.programLocationInfo.exeName

    std::string formatTextParas( std::string text, std::string::size_type paraWidth
                               , TextAlignment textAlignment // = TextAlignment::width
                               , const SymbolLenCalculator &symbolLenCalculator // = SymbolLenCalculatorEncodingSingleByte()
                               , bool bAppendMissingComma=true
                               )
    inline
    std::string formatTextParas( std::string text, std::string::size_type paraWidth
                               , TextAlignment textAlignment // = TextAlignment::width
                               , bool bAppendMissingComma=true
                               )


    */


protected:

    struct CommandHelpInfo
    {
        std::string            commandName;     // Полное имя - собираем по мере обхода
        std::string            usageInfo;       // Помещается из тупикового (листового) элемента
        std::string            brief;           // Помещается из тупикового (листового) элемента
        std::string            description;     // Помещается из тупикового (листового) элемента
        std::set<std::string>  allowedOptions;  // Собираем по мере спуска


        std::string makeUsageString(const std::string &exeName, bool hasGlobalOptions) const
        {
            auto fullCmd = exeName + " ";

            if (hasGlobalOptions)
                fullCmd += "[GLOBAL_OPTIONS...] ";

            fullCmd += commandName;

            if (!usageInfo.empty())
            {
                fullCmd += " ";
                fullCmd += usageInfo;
            }

            return fullCmd;
        }

        std::string makeCommandOptionHelpCommand(const std::string &exeName, const std::string &optForHelp) const
        {
            auto fullCmd = exeName + " ";

            fullCmd += commandName;

            fullCmd += " ";
            fullCmd += optForHelp;
            fullCmd += " --help";

            return fullCmd;
        }

        std::string makeCommandHelpCommand(const std::string &exeName) const
        {
            auto fullCmd = exeName + " ";

            fullCmd += commandName;

            fullCmd += " --help";

            return fullCmd;
        }



    };

    // Рекурсивно обходит всех детей. Добавляет собранное в вектор, когда у узла нет детей
    // Если у узла нет имени, то allowedOptions не обновляются - глобальные опции пропускаются
    template<typename IterType>
    static
    void collectCommandHelpInfoImpl( const CommandInfo             *pCommandInfo
                                   , IterType                      beginSeqIt
                                   , IterType                      endSeqIt
                                   , CommandHelpInfo               collectingHelpInfo
                                   , std::vector<CommandHelpInfo>  &collectToVec
                                   , std::size_t                   curLevel = 0
                                   )
    {
        if (!pCommandInfo->commandName.empty())
        {
            collectingHelpInfo.allowedOptions.insert(pCommandInfo->allowedOptions.begin(), pCommandInfo->allowedOptions.end());

            if (curLevel)
            {
                if (!collectingHelpInfo.commandName.empty())
                    collectingHelpInfo.commandName.append(1, ' ');
    
                collectingHelpInfo.commandName.append(pCommandInfo->commandName);
            }
        }

        if (pCommandInfo->subCommands.empty())
        {
            collectingHelpInfo.usageInfo   = pCommandInfo->usageInfo  ;
            collectingHelpInfo.brief       = pCommandInfo->brief      ;
            collectingHelpInfo.description = pCommandInfo->description;

            // usageInfo может быть пустым, так-то. Это факт, а не лень заполнения, такое бывает, когда у команды нет никаких опций и операндов
            // 
            // #if !defined(UMBA_CMD_LINE_CMD_CONTROLLER_NO_THROW_ON_EMPTY_CMD_USAGE_INFO)
            //     if (collectingHelpInfo.usageInfo.empty())
            //         throw std::runtime_error("umba::cmd_line::CommandSequenceController::collectCommandHelpInfoImpl: usageInfo not set for command: '" + collectingHelpInfo.commandName + "'");
            // #endif

            // Если у команды есть опции, то если usageInfo пустое или не начинается с символа '[', то вставляем "[OPTIONS...]" или "[ИМЯ_КОМАНДЫ_OPTIONS...]"
            if ( !collectingHelpInfo.allowedOptions.empty()
              && (collectingHelpInfo.usageInfo.empty() || collectingHelpInfo.usageInfo.front()!='[')
               )
            {
                std::string optionsPrefix = pCommandInfo->commandName;
                string::toupper(optionsPrefix);
                if (!optionsPrefix.empty())
                    optionsPrefix.append(1, '_');
                optionsPrefix.append("OPTIONS...");

                optionsPrefix.insert(0, 1, '[');
                optionsPrefix.append(1, ']');

                if (!collectingHelpInfo.usageInfo.empty())
                    optionsPrefix.append(1, ' ');

                collectingHelpInfo.usageInfo.insert(0, optionsPrefix);
            }
            
            // Краткое описание не может быть пустым
            #if !defined(UMBA_CMD_LINE_CMD_CONTROLLER_NO_THROW_ON_EMPTY_CMD_BRIEF)
                if (collectingHelpInfo.brief.empty())
                    throw std::runtime_error("umba::cmd_line::CommandSequenceController::collectCommandHelpInfoImpl: brief not set for command: '" + collectingHelpInfo.commandName + "'");
            #endif

            // Описание может быть пустым, хрен с ним
            // #if !defined(UMBA_CMD_LINE_CMD_CONTROLLER_NO_THROW_ON_EMPTY_CMD_DESCRIPTION)
            //     if (collectingHelpInfo.description.empty())
            //         throw std::runtime_error("umba::cmd_line::CommandSequenceController::collectCommandHelpInfoImpl: description not set for command: '" + collectingHelpInfo.commandName + "'");
            // #endif
            
            collectToVec.push_back(collectingHelpInfo);

            return;
        }


        auto nextBegin = beginSeqIt;
        if (nextBegin!=endSeqIt)
            nextBegin = std::next(nextBegin);

        for(const auto &[subCmdKeyName, subCmdInfo] : pCommandInfo->subCommands)
        {
            if (subCmdInfo.commandName.empty())
                throw std::runtime_error("umba::cmd_line::CommandSequenceController::collectCommandHelpInfoImpl: something goes wrong - commandName not set");

            if (nextBegin!=endSeqIt)
            {
                if (subCmdInfo.commandName!=*nextBegin)
                    continue;
            }

            collectCommandHelpInfoImpl( &subCmdInfo
                                      , nextBegin, endSeqIt
                                      , collectingHelpInfo, collectToVec, curLevel+1
                                      );
        }

    }


    std::vector<CommandHelpInfo> collectCommandHelpInfo() const
    {
        auto cmdSeq = commandSequence;
        cmdSeq.insert(cmdSeq.begin(), std::string());

        std::vector<CommandHelpInfo> resVec;
        collectCommandHelpInfoImpl( &commandInfo
                                  , cmdSeq.begin()
                                  , cmdSeq.end()
                                  , CommandHelpInfo()
                                  , resVec
                                  , 0
                                  );
        return resVec;
    }

    static
    void collectCommandNamesImpl(const CommandInfo *pCommandInfo, std::set<std::string> &collectTo) 
    {
        {
            auto cmdNameAsVec = pCommandInfo->splitCommandStr(pCommandInfo->commandName);
            collectTo.insert(cmdNameAsVec.begin(), cmdNameAsVec.end());
        }

        for(const auto &[subCmdName, cmdInfo] : pCommandInfo->subCommands)
        {
            collectCommandNamesImpl(&cmdInfo, collectTo);
        }
    }

    std::set<std::string> collectCommandNames() const
    {
        std::set<std::string> s;
        collectCommandNamesImpl(&commandInfo, s);
        return s;
    }


    enum class BE {begin, end};

    static std::string s_code_(PrintHelpStyle helpStyle, BE be)
    {
        if (helpStyle==PrintHelpStyle::wiki)
            return be==BE::begin ? "<code>\n" : "</code>\n";
        else if (helpStyle==PrintHelpStyle::md)
            return "```\n";
        else
            return be==BE::begin ? "    " : "";
    }

    static std::string s_codeNext_(PrintHelpStyle helpStyle, BE be)
    {
        if (helpStyle==PrintHelpStyle::wiki)
            return be==BE::begin ? "" : "\n"; // std::string();
        else if (helpStyle==PrintHelpStyle::md)
            return be==BE::begin ? "" : "\n"; // std::string();
        else
            return be==BE::begin ? "    " : "\n";
    }

    static std::string s_inline_code_(PrintHelpStyle helpStyle, BE  /* be */ )
    {
        if (helpStyle==PrintHelpStyle::wiki)
            return "\'\'"; // be==BE::begin ? "<code>\n" : "</code>\n";
        else if (helpStyle==PrintHelpStyle::md)
            return "`";
        else
            return "";
    }


    static std::string s_em_(PrintHelpStyle helpStyle, BE be)
    {
        if (helpStyle==PrintHelpStyle::wiki)
            return be==BE::begin ? "**%%" : "%%**";
        else if (helpStyle==PrintHelpStyle::md)
            return "**";
        else
            return std::string();
    }

    static std::string s_tt_(PrintHelpStyle helpStyle, BE be)
    {
        return s_inline_code_(helpStyle, be);
        // if (helpStyle==PrintHelpStyle::wiki)
        //     return be==BE::begin ? "**%%" : "%%**";
        // else if (helpStyle==PrintHelpStyle::md)
        //     return "**";
        // else
        //     return std::string();
    }

    static std::string s_listItem_(PrintHelpStyle helpStyle, BE be)
    {
        if (helpStyle==PrintHelpStyle::wiki)
            return be==BE::begin ? "* " : "";
        else if (helpStyle==PrintHelpStyle::md)
            return be==BE::begin ? "- " : "";
        else
            return be==BE::begin ? "    " : "";
    }

    static std::string s_heading_(PrintHelpStyle helpStyle, BE be, std::size_t level)
    {
        if (level<1)
            level = 1;

        // Wiki
        // ====== Headline Level 1 ======
        // ===== Headline Level 2 =====
        // ==== Headline Level 3 ====
        // === Headline Level 4 ===
        // == Headline Level 5 ==
        // 6 - level + 1 // level: 1-5

        // MD
        // # Headline Level 1
        // ## Headline Level 2
        // ### Headline Level 3

        if (helpStyle==PrintHelpStyle::wiki)
        {
            if (level>5)
                level = 5;

            return be==BE::begin
                 ? std::string(6-level+1, '=') + " "
                 : " " + std::string(6-level+1, '=')
                 ;
        }
        else if (helpStyle==PrintHelpStyle::md)
        {
            if (level>6)
                level = 6;

            return be==BE::begin
                 ? std::string(level, '#') + " "
                 : std::string()
                 ;
        }
        else
        {
            return be==BE::begin
                 ? std::string()
                 : ":"
                 ;
        }
    }


    static std::string s_code(PrintHelpStyle helpStyle, const std::string &str)
    {
        auto resStr = s_code_(helpStyle, BE::begin);
        //bool needLf = !resStr.empty();
        //return resStr + str + (needLf ? "\n" : "") + code(BE::end);
        return resStr + str + "\n" + s_code_(helpStyle, BE::end);
    };

    static std::string s_inline_code(PrintHelpStyle helpStyle, const std::string &str)
    {
        auto resStr = s_inline_code_(helpStyle, BE::begin);
        //bool needLf = !resStr.empty();
        //return resStr + str + (needLf ? "\n" : "") + code(BE::end);
        return resStr + str + s_inline_code_(helpStyle, BE::end);
    };

    static std::string s_em(PrintHelpStyle helpStyle, const std::string &str)
    {
        return s_em_(helpStyle, BE::begin) + str + s_em_(helpStyle, BE::end);
    }

    static std::string s_tt(PrintHelpStyle helpStyle, const std::string &str)
    {
        return s_tt_(helpStyle, BE::begin) + str + s_em_(helpStyle, BE::end);
    }

    static std::string s_listItem(PrintHelpStyle helpStyle, const std::string &str)
    {
        return s_listItem_(helpStyle, BE::begin) + str + s_listItem_(helpStyle, BE::end) + "\n";
    }

    static std::string s_heading(PrintHelpStyle helpStyle, const std::string &str, std::size_t level)
    {
        return s_heading_(helpStyle, BE::begin, level) + str + s_heading_(helpStyle, BE::end, level) + "\n";
    }

    static bool s_isRichFormat(PrintHelpStyle helpStyle)        { return helpStyle!=PrintHelpStyle::normal; }
    static bool s_isCompletionFormat(PrintHelpStyle helpStyle)  { return helpStyle==PrintHelpStyle::bash_complete || helpStyle==PrintHelpStyle::clink_complete; }


    static void s_printHelpOnHelp(std::ostream &oss, PrintHelpStyle helpStyle, const std::string &exeName)
    {
        //if (helpOnHelp)
        {
            if (s_isRichFormat(helpStyle)) // Требует отдельного раздела
                oss << s_heading(helpStyle, "Getting help", 2) << "\n";

            oss << s_heading(helpStyle, "Help options", 3) << "\n"; // Help options
            oss << s_listItem(helpStyle, s_em(helpStyle, "-?"));
            oss << s_listItem(helpStyle, s_em(helpStyle, "-h"));
            oss << s_listItem(helpStyle, s_em(helpStyle, "--help"));
            oss << "\n";

            oss << s_heading(helpStyle, "Get All Help", 3) << "\n"; // All help
            oss << s_code(helpStyle, exeName + " --help") << "\n";

            oss << s_heading(helpStyle, "Get Help on Option(s)", 3) << "\n"; // Help on option(s)
            oss << s_code(helpStyle, exeName + " OPTION... --help") << "\n";

            oss << s_heading(helpStyle, "Get Help on Command", 3) << "\n"; // Help on command
            oss << s_code(helpStyle, exeName + " COMMAND [SUBCOMMAND...] --help") << "\n";

            oss << s_heading(helpStyle, "Get Help on Command Option(s)", 3) << "\n"; // Help on command option(s)
            oss << s_code(helpStyle, exeName + " COMMAND [SUBCOMMAND...] OPTION... --help") << "\n";
            
        }
    }

    template<typename ResultSetType, typename SetType1, typename SetType2>
    static ResultSetType setIntersect(const SetType1 &s1, const SetType2 &s2)
    {
        ResultSetType resSet;

        for(auto &&sm1 : s1)
        {
            if (s2.find(sm1)==s2.end())
                continue;

            resSet.insert(sm1);
        }

        return resSet;
    }


public:

    std::string makeHelp( std::size_t                  textWidth
                        , ICommandLineOptionCollector  *pCol
                        , const std::set<std::string>  &argsNeedHelp
                        , bool                         helpOnHelp = true
                        , std::string                  exeName = std::string()
                        ) const
    {
        UMBA_ARG_USED(textWidth);
        UMBA_ARG_USED(pCol);
        UMBA_ARG_USED(argsNeedHelp);
        UMBA_ARG_USED(exeName);

        if (exeName.empty())
            exeName = pCol->getExeName(false /* !useExt */ );

        if (textWidth<24)
            textWidth = 24;


        std::ostringstream oss;

        auto helpStyle = pCol->getPrintHelpStyle();
        // std::string pCol->getExeName(bool useExt)

        auto isRichFormat = [&]()       { return s_isRichFormat(helpStyle); };
        auto isCompletionFormat = [&]() { return s_isCompletionFormat(helpStyle); };

        auto plainIndent = [&](std::size_t ind) -> std::string
        {
            if (isRichFormat())
                return std::string();
            else
                return std::string(ind*4, ' ');
        };

        //--------------------------------------------------------------------------------------------------------------------
        if (isCompletionFormat())
        {
            // Пока сделаем по простому, через complete -W
            // https://chat.deepseek.com/share/6mwr7tvh9agfqd61sl
            // --bash : complete -W " -q --quet --home --no-builtin-options --no-custom-builtin-options --no-user-builtin-options -v --version --version-info --builtin-options-info --where --color --gcc --autocomplete-install --autocomplete-uninstall -Y --overwrite -N --no-overwrite --role-setup -d --delete -D --force-delete -m --move -a --all --message --global --local -l --list --unset --oneline --graph -p --patch --since -S --soft -X --mixed -H --hard --init -R --recursive" _test08.exe
            // --clink: local _test08.exe_tree = { "-q", "--quet", "--home", "--no-builtin-options", "--no-custom-builtin-options", "--no-user-builtin-options", "-v", "--version", "--version-info", "--builtin-options-info", "--where", "--color", "--gcc", "--autocomplete-install", "--autocomplete-uninstall", "-Y", "--overwrite", "-N", "--no-overwrite", "--role-setup", "-d", "--delete", "-D", "--force-delete", "-m", "--move", "-a", "--all", "--message", "--global", "--local", "-l", "--list", "--unset", "--oneline", "--graph", "-p", "--patch", "--since", "-S", "--soft", "-X", "--mixed", "-H", "--hard", "--init", "-R", "--recursive" } clink.arg.register_parser("_test08.exe", _test08.exe_tree)

            // TODO: Надо сделать через функцию: complete -F

            // В старом коде переделывать что-либо не охота, поэтому тупо встроимся в выдачу
            static const std::string bashPrefix  = "complete -W \"";
            static const std::string clinkPrefix = "_tree = { ";

            std::set<std::string> commandNamesSet = collectCommandNames();

            std::string helpText = pCol->makeText(textWidth, &argsNeedHelp);

            auto pos = helpText.find(bashPrefix);
            if (pos != helpText.npos)
            {
                pos += bashPrefix.size();
                std::string commandNames = string::merge<std::string>(commandNamesSet.begin(), commandNamesSet.end(), ' ');
                helpText.insert(pos, commandNames);
            }
            else if ((pos=helpText.find(clinkPrefix)) != helpText.npos)
            {
                pos += clinkPrefix.size();
                std::string commandNames = " ";
                for(auto &&cmdName: commandNamesSet)
                {
                    commandNames.append("\"" + cmdName + "\", ");
                }
                helpText.insert(pos, commandNames);
            }

            return helpText;
        }


        //--------------------------------------------------------------------------------------------------------------------
        auto code_          = [&](BE be) -> std::string                     { return s_code_(helpStyle, be); };
        auto inline_code_   = [&](BE be) -> std::string                     { return s_inline_code_(helpStyle, be); };
        auto codeNext_      = [&](BE be) -> std::string                     { return s_codeNext_(helpStyle, be); };
        auto em_            = [&](BE be) -> std::string                     { return s_em_(helpStyle, be); };
        auto tt_            = [&](BE be) -> std::string                     { return s_tt_(helpStyle, be); };
        auto listItem_      = [&](BE be) -> std::string                     { return s_listItem_(helpStyle, be); };
        auto heading_       = [&](BE be, std::size_t level) -> std::string  { return s_heading_(helpStyle, be, level); };
        auto code           = [&](const std::string &str) -> std::string    { return s_code(helpStyle, str); };
        auto inline_code    = [&](const std::string &str) -> std::string    { return s_inline_code(helpStyle, str); };
        auto em             = [&](const std::string &str) -> std::string    { return s_em(helpStyle, str); };
        auto tt             = [&](const std::string &str) -> std::string    { return s_tt(helpStyle, str); };
        auto listItem       = [&](const std::string &str) -> std::string    { return s_listItem(helpStyle, str); };
        auto heading        = [&](const std::string &str, std::size_t level) -> std::string { return s_heading(helpStyle, str, level); };


        auto formatParas = [&](const std::string &paras, std::size_t indend)
        {
            auto w = textWidth;
            if (w>indend)
                w -= indend;

            if (w<32)
                w = 32;

            auto formattedParas = umba::text_utils::formatTextParas( paras, w, umba::text_utils::TextAlignment::left );

            if (!isRichFormat())
            {
                formattedParas = umba::text_utils::textAddIndent(formattedParas, std::string(indend, ' '));
            }

            return formattedParas;
        };



        //--------------------------------------------------------------------------------------------------------------------
        auto printHelpOnHelp = [&]()
        {
            if (helpOnHelp)
                s_printHelpOnHelp(oss, helpStyle, exeName);
        };

        //--------------------------------------------------------------------------------------------------------------------
        std::vector<CommandHelpInfo> collectedHelpInfo = collectCommandHelpInfo();

        if (collectedHelpInfo.empty())
        {
            // oss << "makeHelp: something goes wrong\n";
            throw std::runtime_error("umba::cmd_line::CommandSequenceController::makeHelp: something goes wrong");
        }

        std::set<std::string> globalAllowedOptions = std::set<std::string>(commandInfo.allowedOptions.begin(), commandInfo.allowedOptions.end());



        if (commandSequence.empty() && !argsNeedHelp.empty())
            return pCol->makeText( textWidth, &argsNeedHelp );


        //if (commandSequence.empty() && !isRichFormat())
            oss << "Usage:\n\n";

        if (commandSequence.empty()) // выводим глобальную usage info если не задано подкоманд
        {
            auto usageString = commandInfo.usageInfo;
            if (usageString.empty())
            {
                if (commandInfo.subCommands.empty())
                {
                    if (globalAllowedOptions.empty())
                        usageString = "[PARAMETERS...]";
                    else
                        usageString = "[OPTIONS...] [PARAMETERS...]";
                }
                else // Есть подкоманды
                {
                    if (!globalAllowedOptions.empty())
                        usageString = "[GLOBAL_OPTIONS...] COMMAND [SUBCOMMAND...] [COMMAND_OPTIONS...] [PARAMETERS...]";
                    else
                        usageString = "COMMAND [SUBCOMMAND...] [COMMAND_OPTIONS...] [PARAMETERS...]";
                }
            }

            oss << code(exeName + " " + usageString) << "\n";
        }

        if (commandSequence.empty()) // выводим справку по справке только если у нас не задано подкоманд
        {
            printHelpOnHelp();
            oss << "\n";
        }

        if (!commandInfo.subCommands.empty())
        {
            
            if (commandSequence.empty() || collectedHelpInfo.size()>1)
            {
                // Выводим summary по командам, если у нас есть набор подкоманд (больше одной), по которым надо вывести справку
                if (commandSequence.empty())
                {
                    oss << heading("Commands Summary", 2); // << "\n";
                    oss << "\n";
                }

                // std::vector<CommandHelpInfo> collectedHelpInfo = collectCommandHelpInfo();

                oss << code_(BE::begin);
                for(auto it=collectedHelpInfo.begin(); it!=collectedHelpInfo.end(); ++it)
                {
                    auto fullCmd = it->makeUsageString(exeName, !globalAllowedOptions.empty());

                    if (it!=collectedHelpInfo.begin())
                        oss << codeNext_(BE::begin);
                    oss << fullCmd;
                    oss << codeNext_(BE::end);
                }
                oss << code_(BE::end);

                oss << "\n";
            }


            if (commandSequence.empty() && !globalAllowedOptions.empty())
            {
                // Выводим глобальные опции, если у нас нет подкоманд, по которым требуется справка
                oss << "\n";
                oss << heading("Global options", 2); // << "\n";
                // oss << "\n"; // тут не нужно
                oss << pCol->makeText( textWidth, &globalAllowedOptions ); // выводим справку по глобальным опциям
                oss << "\n";
                oss << "\n";
                //oss << "\n";
            }


            if (commandSequence.empty() || collectedHelpInfo.size()>1)
            {
                //if (s_isRichFormat(helpStyle)) // Требует отдельного раздела. Или нет?
                    oss << heading("Commands", 2) << "\n";
                // oss << "\n";
            }


            for(auto it=collectedHelpInfo.begin(); it!=collectedHelpInfo.end(); ++it)
            {
                if (commandSequence.empty() || collectedHelpInfo.size()>1)
                    oss << heading("'" + it->commandName + "' command", 3) << "\n";

                static const std::size_t sz24 = 16;

                std::string strUsage;

                oss << code_(BE::begin);
                    // oss << codeNext_(BE::begin);
                    strUsage = it->makeUsageString(exeName, !globalAllowedOptions.empty());
                    oss << strUsage;
                oss << codeNext_(BE::end);

                if (commandSequence.empty())
                {
                    oss << codeNext_(BE::begin);
                        strUsage = it->makeCommandHelpCommand(exeName);
                        // oss << plainIndent(1) << strUsage;
                        oss << strUsage;
                    oss << codeNext_(BE::end);
                }
                oss << code_(BE::end);

                oss << "\n";


                std::size_t infoIndendCommon = 4;

                std::size_t infoIndend = 0;
                if (collectedHelpInfo.size()>1)
                    infoIndend = 1;

                if (it->brief.empty() || it->description.empty())
                {
                    infoIndend = 0;
                    if (collectedHelpInfo.size()<2)
                        infoIndendCommon = 0;
                }


                if (!it->brief.empty())
                {
                    if (!commandSequence.empty())
                    {
                        if (!it->brief.empty() && !it->description.empty())
                            oss << plainIndent(infoIndend) << heading("Brief description", 6) << "\n";
                    }
                    oss << formatParas(it->brief, 4*infoIndend+infoIndendCommon) << "\n";
                    oss << "\n";
                }

                if (!commandSequence.empty())
                {
                    if (!it->description.empty())
                    {
                        if (!it->brief.empty() && !it->description.empty())
                            oss << plainIndent(infoIndend) << heading("Detailed description", 6) << "\n";
                        oss << formatParas(it->description, 4*infoIndend+infoIndendCommon) << "\n";
                        oss << "\n";
                    }
                }

                
                auto commandOptions = std::set<std::string>(it->allowedOptions.begin(), it->allowedOptions.end());

                //const std::set<std::string>  &argsNeedHelp

                if (commandSequence.empty())
                {
                    // У нас вывод по всем командам

                    if (!commandOptions.empty())
                    {
                        oss << plainIndent(1) << heading("Command options", 6) << "\n";
                        for(const auto &optName : commandOptions)
                        {
                            auto optFullName = std::string(optName.size()>1 ? "--" : "-") + optName;
                            auto fillingSize = optFullName.size()>sz24 ? std::size_t(0) : sz24 - optFullName.size();
                            auto fillStr = std::string(fillingSize, ' ');

                            // !!! Пока не даём подсказку по вызову хелпа конкретной опции конкретной команды,
                            // потому что этот механихм пока не работает, и всегда выводится информация по всем опциям команды
                            auto opInfoFullStr = optFullName; // + "    " + fillStr + it->makeCommandOptionHelpCommand(exeName, optFullName);

                            auto optPrintText = listItem(inline_code(opInfoFullStr));

                            //!!! Нужна склейка двух inline блоков кода
                            // auto optPrintText = listItem(inline_code(optFullName) + tt("    " + fillStr) + it->makeCommandOptionHelpCommand(exeName, optFullName) );
                            oss << (isRichFormat() ? "" : "    ") << optPrintText; // << "\n";
                        }

                        oss << "\n";
                    }
                }
                else
                {
                    // !!! Для конкретной команды у нас почему-то опции не собираются
                    // Пока и так сойдёт, на потом надо разобраться

                    auto printHelpForOptions = argsNeedHelp;   // Хотим выводить инфу только по тем опциям, которые явно указаны в командой строке
                    if (printHelpForOptions.empty())           // Но нам явно не указали, какие опции интересуют
                        printHelpForOptions = commandOptions;  // Значит, выводим инфу по всем опциям данной команды

                    if (collectedHelpInfo.size()>1)
                        printHelpForOptions.clear();

                    if (!printHelpForOptions.empty())
                    {
                        //if (printHelpForOptions.size()>1 || collectedHelpInfo.size()>1)
                        if (printHelpForOptions.size()>1)
                            oss << plainIndent(0) << heading("Command options", 6); // << "\n";

                        oss << pCol->makeText( textWidth, &printHelpForOptions );
                        oss << "\n";
                    }

                }

            } // for(auto it=collectedHelpInfo.begin(); it!=collectedHelpInfo.end(); ++it)

        } // if (!commandInfo.subCommands.empty())



    // struct CommandHelpInfo
    // {
    //     std::string            commandName;     // Полное имя - собираем по мере обхода
    //     std::string            usageInfo;       // Помещается из тупикового (листового) элемента
    //     std::string            brief;           // Помещается из тупикового (листового) элемента
    //     std::string            description;     // Помещается из тупикового (листового) элемента
    //     std::set<std::string>  allowedOptions;  // Собираем по мере спуска
    // };

        // exeName
        // EXE [GLOBAL_OPTIONS...] COMMAND [SUBCOMMAND...] [COMMAND_OPTIONS...] [PARAMETERS...]  - главная строка, прошита (или берём из корня, если там задано)
        // helpOnHelp


// struct CommandInfo
// {
//     std::string                          commandName;
//     std::string                          usageInfo;
//     std::string                          brief;
//     std::string                          description;
//     std::map<std::string, CommandInfo>   subCommands;
//     std::unordered_set<std::string>      allowedOptions;
// };
//  
//  
// struct CommandSequenceController
// {
//     CommandInfo                   commandInfo; // 
//     mutable bool                  bSealed = false; // Запечатывание происходит, когда после команды или нескольких приходит опция
//     std::vector<std::string>      commandSequence;
//     std::vector<std::string>      inputList;
// };


        return oss.str();


    }


}; // struct CommandSequenceController

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace command_line
} // namespace umba

// umba::command_line::



