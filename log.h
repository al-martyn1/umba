/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Вывод в лог (на консоль), хелперы

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once


#if !defined(UMBA_SIMPLE_FORMATTER_H)
    #error "umba/log.h requires umba/simple_formatter.h to be included first"
#endif

#include <string>
#include <unordered_map>
#include <cstdint>


// requires global vars
extern bool umbaLogGccFormat  ;
extern bool umbaLogSourceInfo ;

extern umba::SimpleFormatter  umbaLogStreamErr;
extern umba::SimpleFormatter  umbaLogStreamMsg;
extern umba::SimpleFormatter  umbaLogStreamNul;

//! Возвращает umbaLogStreamMsg/umbaLogStreamNul в зависимости от запрошенного уровня (level) verbosity и текущих настроек программы. В простейшем случае просто проверяется флаг quet
extern umba::SimpleFormatter& getLogMsgStream(int level); // =0

// Если надо будет протоколировать в какой-то отдельный поток вывода в каком-то скопе,
// то там надо будет как-то извращаться. Но пока это обычно для утилит командной строки,
// и достаточно глобальных переменных


// source parsing errors
// requires 
//   std::string curFile
//   unsigned lineNo
// in log scope
#define UMBA_LOG_ERR_INPUT                 umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::err ,  /* gopts, */  std::string()/*"err"*/, curFile.c_str(), lineNo, __FILE__, __LINE__ )
#define UMBA_LOG_ERR_INPUT_EX(errType)     umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::err ,  /* gopts, */  std::string(errType)  , curFile.c_str(), lineNo, __FILE__, __LINE__ )
#define UMBA_LOG_WARN_INPUT(warnType)      umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::warn,  /* gopts, */  std::string(warnType) , curFile.c_str(), lineNo, __FILE__, __LINE__ )
#define UMBA_LOG_INFO_INPUT(infoType)      umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::msg ,  /* gopts, */  std::string(infoType) , curFile.c_str(), lineNo, __FILE__, __LINE__ )

// options and other errors
#define UMBA_LOG_ERR                       umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::err ,  /* gopts,  */ std::string()/*"err"*/, (const char*)0 , 0     , __FILE__, __LINE__ )
#define UMBA_LOG_WARN(warnType)            umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::warn,  /* gopts,  */ std::string(warnType) , (const char*)0 , 0     , __FILE__, __LINE__ )
#define UMBA_LOG_INFO(infoType)            umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::msg ,  /* gopts,  */ std::string(infoType) , (const char*)0 , 0     , __FILE__, __LINE__ )
                                                                            /*         */
#define UMBA_LOG_MSG                       umba::log::startLogError( umbaLogStreamErr, umba::log::LogEntryType::msg ,  /* gopts,  */ std::string()/*"msg"*/, (const char*)0 , 0     , (const char*)0, 0 )




// umba::log::
namespace umba {
namespace log {



enum class LogEntryType
{
    err, warn, msg
};


//----------------------------------------------------------------------------
inline
std::string makeAllWarnInfoLogOptionsString(const std::set<std::string> &s)
{
    auto stringifier = [](const std::string &str)
    {
        return "`" + str + "`";
    };

    return umba::string_plus::merge<std::string, std::set<std::string>::const_iterator, decltype(stringifier)>( s.begin(), s.end(), ", ", stringifier);
}

//----------------------------------------------------------------------------
inline
bool addRemoveLogOptionsImpl( std::unordered_map<std::string, bool>& optDisabledMap
                            , const std::set<std::string> &allOpts
                            , const std::string &optString
                            , std::string &unknownOpt
                            )
{
    std::vector<std::string> optsVec = umba::string_plus::split(optString, ',');
    // marty_cpp::splitToLinesSimple(optString, false, ',');

    for(auto opt: optsVec)
    {
        bool bRemove = false; // remove from disabled (add to enabled)

        umba::string_plus::trim(opt, umba::string_plus::is_one_of<char>(" \t"));
        if (opt.empty())
            continue;

        if (opt[0]=='+' || opt[0]=='-')
        {
            bRemove = opt[0]=='-';
            opt.erase(0, 1);
        }

        umba::string_plus::trim(opt, umba::string_plus::is_one_of<char>(" \t"));
        if (opt.empty())
            continue;

        if (opt=="all")
        {
            for(const auto &optFromAll: allOpts)
            {
                optDisabledMap[optFromAll] = bRemove;
            }
        }
        else
        {
            if (allOpts.find(opt)==allOpts.end())
            {
                unknownOpt = opt;
                return false;
            }

            optDisabledMap[opt] = bRemove;
        }
    }

    return true;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::unordered_map<std::string, bool>& getWarningDisabledMap()
{
    static std::unordered_map<std::string, bool> m;
    return m;

    // В многопотоке могут быть проблемы.
    // С другой стороны, все настройки варнингов делаются обычно до какой-то работы
    // и в многопотоке будет только поиск
}

inline
bool isWarningDisabled(const std::string &warnType)
{
    const std::unordered_map<std::string, bool> &m = getWarningDisabledMap();
    std::unordered_map<std::string, bool>::const_iterator it = m.find(warnType);
    if (it==m.end())
        return false; // not explicitly disabled
    return it->second;
}

inline
void setWarningDisabled(const std::string &warnType, bool bDisabled=true)
{
    std::unordered_map<std::string, bool> &m = getWarningDisabledMap();
    m[warnType] = bDisabled;
}

//----------------------------------------------------------------------------
inline
bool addRemoveWarningOptions( const std::set<std::string> &allOpts
                            , const std::string &optString
                            , std::string &unknownOpt
                            )
{
    return addRemoveLogOptionsImpl( getWarningDisabledMap(), allOpts, optString, unknownOpt );
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::unordered_map<std::string, bool>& getInfoDisabledMap()
{
    static std::unordered_map<std::string, bool> m;
    return m;

    // В многопотоке могут быть проблемы.
    // С другой стороны, все настройки варнингов делаются обычно до какой-то работы
    // и в многопотоке будет только поиск
}

inline
bool isInfoDisabled(const std::string &warnType)
{
    const std::unordered_map<std::string, bool> &m = getInfoDisabledMap();
    std::unordered_map<std::string, bool>::const_iterator it = m.find(warnType);
    if (it==m.end())
        return false; // not explicitly disabled
    return it->second;
}

inline
void setInfoDisabled(const std::string &warnType, bool bDisabled=true)
{
    std::unordered_map<std::string, bool> &m = getInfoDisabledMap();
    m[warnType] = bDisabled;
}

//----------------------------------------------------------------------------
inline
bool addRemoveInfoOptions( const std::set<std::string> &allOpts
                         , const std::string &optString
                         , std::string &unknownOpt
                         )
{
    return addRemoveLogOptionsImpl( getInfoDisabledMap(), allOpts, optString, unknownOpt );
}

//----------------------------------------------------------------------------








//----------------------------------------------------------------------------
inline
umba::SimpleFormatter& startLogError( umba::SimpleFormatter     &s
                                    , LogEntryType              logEntryType
                                    // , const GeneratorOptions &gopts
                                    , const std::string         &warnType
                                    , const char* inputFile   , std::size_t inputLineNo
                                    , const char* srcFile = 0 , int srcLineNo = 0 /* Компилятор генерит int конствнту при использовании __LINE__ */
                                    )
{
    if ( (logEntryType==LogEntryType::warn && isWarningDisabled(warnType))
      || (logEntryType==LogEntryType::msg  && isInfoDisabled(warnType) && !warnType.empty())
       )
        return umbaLogStreamNul;


    using namespace umba::omanip;

    if (logEntryType==LogEntryType::err || logEntryType==LogEntryType::warn)
    {
        if (logEntryType==LogEntryType::err)
            s<<error;
        else
            s<<warning;
    }

    if (inputFile)
    {
        if (umbaLogGccFormat)
        {
            // ..\src\main\payloads_bus_master_task.h:60:1: error:
            if (inputFile) //-V547
                s<<inputFile<<":"<<inputLineNo<<": ";
        }
        else
        {
            // e:\_work\utils\rdlc\src\rdlc.cpp(668): error C2065:
            if (inputFile) //-V547
                s<<inputFile<<"("<<inputLineNo<<"): ";
        }

        if (logEntryType==LogEntryType::err)
        {
            s<< "error";
            if (!warnType.empty())
            {
                // also need to check warn is enabled or not
                s<< "("<<warnType<<")";
            }
            s<< ":";
        }
        else if (logEntryType==LogEntryType::warn)
        {
            s<< "warning";
            if (!warnType.empty())
            {
                // also need to check warn is enabled or not
                s<< "("<<warnType<<")";
            }
            // if (logWarnType)
            //     s<< "("<<warnType<<"):";
            s<< ":";
        }
        else if (logEntryType==LogEntryType::msg && !warnType.empty())
        {
            s<< "info";
            s<< "("<<warnType<<")";
            s<< ":";
        }
    }
    else
    {
        if (logEntryType==LogEntryType::err)
        {
            s<< "error";
            if (!warnType.empty())
            {
                // also need to check warn is enabled or not
                s<< "("<<warnType<<")";
            }
            s<< ":";
        }
        else if (logEntryType==LogEntryType::warn)
        {
            s<< "warning";
            if (!warnType.empty())
            {
                // also need to check warn is enabled or not
                s<< "("<<warnType<<")";
            }
            // if (logWarnType)
            //     s<< "("<<warnType<<"):";
            s<< ":";
        }
        else if (logEntryType==LogEntryType::msg && !warnType.empty())
        {
            s<< "info";
            s<< "("<<warnType<<")";
            s<< ":";
        }
    }

    if (srcFile && umbaLogSourceInfo)
    {
        s<<" ";
        s<<coloring( UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::blue, umba::term::colors::color_default, true  ,  false,  false ) )
         <<"["<<srcFile<<":"<<srcLineNo<<"]";
    }

    if (logEntryType==LogEntryType::err || logEntryType==LogEntryType::warn || (logEntryType==LogEntryType::msg && !warnType.empty()))
        s<<normal<<" ";

    return s;
}

} // namespace log
} // namespace umba




