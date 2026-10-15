/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Umba-исключения

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

//-----------------------------------------------------------------------------
// Платформенные настройки: нужно подтянуть windows.h (для FormatMessageA)
// и/или cerrno/cstring (для strerror_r) до всего остального.
//-----------------------------------------------------------------------------

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include "utf8.h"          // umba::toUtf8 (UTF-16 -> UTF-8)
    #include "internal/winstrerrorname.h"
#else
    #include <cerrno>
    #include <cstring>
#endif

#include <cstdint>
#include <string>
#include <exception>
#include <iostream>


//-----------------------------------------------------------------------------

/*!
    \ingroup UMBA_CORE
    \addtogroup UMBA_INTERNALS
*/

//#include "char_writers.h"
#include "debug_helpers.h"
#include "string_char_writers.h"
//#include "i_char_writer.h"
#include "umba.h"


// https://habr.com/ru/post/163393/
// https://habr.com/ru/post/163745/

// EXCEPTION_EXECUTE_HANDLER
// EXCEPTION_CONTINUE_SEARCH


#if defined(_MSC_VER)

    #define UMBA_SEH_TRY_BEGIN()                    __try {
    #define UMBA_SEH_TRY_END()                      }

    #define UMBA_SEH_CATCH_BEGIN()                  __except(EXCEPTION_EXECUTE_HANDLER) {
    #define UMBA_SEH_CATCH_END()                    }

    #define UMBA_SEH_CATCH_CONTINUE_SEARCH_BEGIN()  __except(EXCEPTION_CONTINUE_SEARCH) {
    #define UMBA_SEH_CATCH_CONTINUE_SEARCH_END()    }

#else

    #define UMBA_SEH_TRY_BEGIN()                    {
    #define UMBA_SEH_TRY_END()                      }

    #define UMBA_SEH_CATCH_BEGIN()                  {
    #define UMBA_SEH_CATCH_END()                    }

    #define UMBA_SEH_CATCH_CONTINUE_SEARCH_BEGIN()  {
    #define UMBA_SEH_CATCH_CONTINUE_SEARCH_END()    }

#endif

    #define UMBA_SEH_TRY_END_CATCH()                UMBA_SEH_CATCH_BEGIN() \
                                                    UMBA_SEH_CATCH_END()






namespace umba
{


//============================================================================
//
//  Exception — базовый класс для всех исключений библиотеки umba
//
//============================================================================

//----------------------------------------------------------------------------
/*!
    \brief Маркерный базовый класс для всех исключений библиотеки umba.

    Нужен, чтобы была возможность поймать любое «своё» исключение одной
    catch-секцией, не задевая std::exception и его стандартных потомков:

        try { ... }
        catch (const ::umba::Exception &e) { ... }  // любое umba-исключение
        catch (const std::exception   &e) { ... }  // остальные стандартные

    Интерфейс не расширяет std::exception: ни полей, ни методов, ни
    собственного what(). Все содержательные потомки (SystemException,
    FileException и т.п.) сами добавляют нужные данные и переопределяют
    what().

    \note
        - Деструктор не объявлен — виртуальный деструктор наследуется от
          std::exception. Это избавляет от предупреждения C5267
          (deprecated implicit copy ctor из-за user-declared dtor).
        - Класс специально оставлен «пустым» — это только якорь иерархии.
*/
class Exception : public std::exception
{
    // Намеренно пустой: никакого интерфейса не добавляет.
}; // class Exception

//----------------------------------------------------------------------------




//============================================================================
//
//  Системные коды ошибок и получение их текстового описания
//
//============================================================================

//----------------------------------------------------------------------------
//! Платформозависимый тип кода системной ошибки.
//! Под Windows это DWORD (unsigned long, 32 бита),
//! под *nix — int (то, что возвращает errno).
//----------------------------------------------------------------------------
#if defined(_WIN32)
    using SystemErrorCode = DWORD;
#else
    using SystemErrorCode = int;
#endif


//----------------------------------------------------------------------------
//! Получить код последней системной ошибки.
//! Windows: GetLastError(); *nix: errno.
//----------------------------------------------------------------------------
inline
SystemErrorCode getLastSystemError() noexcept
{
#if defined(_WIN32)
    return ::GetLastError();
#else
    return static_cast<SystemErrorCode>(errno);
#endif
}


//----------------------------------------------------------------------------
//! Внутренние хелперы формирования текстового описания.
//----------------------------------------------------------------------------
namespace detail
{

#if defined(_WIN32)

    //! Windows: FormatMessageW + конвертация UTF-16 -> UTF-8 через umba::toUtf8
    //! с обрезкой хвостовых CR/LF/пробелов
    inline
    std::string systemErrorMessageImpl( SystemErrorCode code )
    {
        LPWSTR buffer = nullptr;

        const DWORD len = ::FormatMessageW(
              FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS
            , nullptr
            , code
            , MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
            , reinterpret_cast<LPWSTR>(&buffer)
            , 0
            , nullptr);

        std::string result;

        if (len && buffer)
        {
            // Обрезаем хвостовые CR/LF/пробелы/табы
            std::size_t n = len;
            while (n > 0)
            {
                const wchar_t ch = buffer[n-1];
                if (ch==L'\r' || ch==L'\n' || ch==L' ' || ch==L'\t')
                    --n;
                else
                    break;
            }

            if (n > 0)
            {
                const std::wstring wide(buffer, n);
                result = ::umba::toUtf8(wide);
            }
        }

        if (buffer)
            ::LocalFree(buffer);

        return result;
    }

#else // POSIX

    //! *nix: strerror_r с подстановкой fallback-а
    inline
    std::string systemErrorMessageImpl( SystemErrorCode code )
    {
        char buffer[512];
        buffer[0] = 0;

        #if defined(__GLIBC__) && defined(_GNU_SOURCE)
            // GNU-вариант: возвращает char*
            const char* msg = ::strerror_r(code, buffer, sizeof(buffer));
            if (msg && *msg)
                return std::string(msg);
        #else
            // XSI-вариант: возвращает int (0 — успех)
            if (::strerror_r(code, buffer, sizeof(buffer)) == 0 && buffer[0])
                return std::string(buffer);
        #endif

        // Fallback — не потокобезопасен, но лучше, чем ничего
        const char* msg = std::strerror(code);
        if (msg && *msg)
            return std::string(msg);

        return std::string();
    }

#endif

} // namespace detail


//----------------------------------------------------------------------------
//! Получить текстовое описание системной ошибки по её коду.
//! Если система не дала описания — вернёт пустую строку.
//----------------------------------------------------------------------------
inline
std::string getSystemErrorMessage( SystemErrorCode code )
{
    return detail::systemErrorMessageImpl(code);
}


//----------------------------------------------------------------------------
//! Сформировать итоговое сообщение вида
//!   "<системное описание> (<числовой код>[/<символическое имя>])"
//! Под Windows к числовому коду добавляется символическое имя (если известно),
//! например: "Access is denied. (5/ERROR_ACCESS_DENIED)".
//!
//! \param code Код системной ошибки. Если равен 0 — возвращается пустая строка.
//! \return    Готовая строка либо пустая строка (если code == 0).
//----------------------------------------------------------------------------
inline
std::string formatSystemErrorMessage( SystemErrorCode code )
{
    if (code == 0)
        return std::string();

    std::string sys = getSystemErrorMessage(code);
    if (sys.empty())
        sys = "Unknown system error";

    // Срезаем хвостовую пунктуацию — она будет восстановлена вызывающим кодом
    if (!sys.empty())
    {
        const char lastCh = sys.back();
        if (lastCh=='.' || lastCh=='!' || lastCh=='?')
            sys.pop_back();
    }

    sys.append(" (");

    sys += std::to_string(static_cast<long long>(code));

#if defined(_WIN32)
    auto codeStr = win32::getStrErrorNameStr(code);
    if (!codeStr.empty())
    {
        sys.append(1, '/');
        sys += codeStr;
    }
#endif

    sys.append(1, ')');

    return sys;
}


//----------------------------------------------------------------------------
//! Хелпер: добавляет завершающую точку, если в конце нет . ! ?
//----------------------------------------------------------------------------
inline
std::string ensureTrailingPunctuation( std::string s )
{
    if (!s.empty())
    {
        const char lastCh = s.back();
        if (lastCh!='.' && lastCh!='!' && lastCh!='?')
            s.push_back('.');
    }
    return s;
}


//----------------------------------------------------------------------------
//! Хелпер: срезает завершающую точку/!/?, при необходимости сохраняя её
//! в *pPunkt.
//----------------------------------------------------------------------------
inline
std::string removeTrailingPunctuation( std::string s, std::string *pPunkt=0 )
{
    if (!s.empty())
    {
        const char lastCh = s.back();
        if (lastCh=='.' || lastCh=='!' || lastCh=='?')
        {
            if (pPunkt)
                pPunkt->assign(1, lastCh);
            s.erase(s.size()-1, 1);
        }
    }
    return s;
}




//============================================================================
//
//  SystemException — исключение, ассоциированное с системной ошибкой
//
//============================================================================

//----------------------------------------------------------------------------
/*!
    \brief Базовое исключение для ошибок ОС.

    Хранит код ошибки (GetLastError() под Windows, errno под *nix) и,
    опционально, текстовое уточнение. Если текст не задан явно, то при
    первом вызове what() формируется системное описание ошибки и кэшируется
    во внутреннем буфере.

    Совместим со std::exception (через umba::Exception).

    \note
        - what() объявлена noexcept (требование override для std::exception).
          Любая операция внутри, способная бросить (аллокация, обращение
          к ОС), обёрнута в try/catch: при неудаче возвращается статический
          фолбэк, чтобы не вызвать std::terminate.
        - Ленивое формирование сообщения не является потокобезопасным;
          для исключений это приемлемо.
        - Деструктор не объявляется явно — виртуальный деструктор
          наследуется от std::exception. Это избавляет от предупреждения
          C5267 (deprecated implicit copy ctor из-за user-declared dtor).
        - SystemException — самостоятельная ветвь иерархии. Файловые
          исключения (FileException и его наследники) от него НЕ наследуются,
          т.к. не всякая файловая ошибка является системной.
*/
class SystemException : public Exception
{
public:

    using ErrorCodeType = SystemErrorCode;

    //------------------------------------------------------------------------
    //! По умолчанию берёт код из GetLastError()/errno.
    SystemException() noexcept
        : m_code(getLastSystemError())
    {}

    //------------------------------------------------------------------------
    //! Из кода системной ошибки.
    UMBA_EXPLICIT
    SystemException( ErrorCodeType code ) noexcept
        : m_code(code)
    {}

    //------------------------------------------------------------------------
    //! Из готового текстового сообщения (без кода).
    UMBA_EXPLICIT
    SystemException( std::string msg )
        : m_code(0)
        , m_message(std::move(msg))
    {}

    //------------------------------------------------------------------------
    //! Из кода и уточняющего текста.
    //! what() вернёт текст как есть (уточнение пользователя), а код доступен
    //! через code().
    SystemException( ErrorCodeType code, std::string msg )
        : m_code(code)
        , m_message(std::move(msg))
    {}

    //! Деструктор
    //~SystemException() noexcept override = default;

    //------------------------------------------------------------------------
    //! Текст ошибки. Формируется лениво по коду при первом обращении.
    //!
    //! \note Метод обязан быть noexcept (нельзя ослабить спецификацию
    //!       исключений родителя std::exception), поэтому тело защищено
    //!       try/catch — при неудаче возвращается статический фолбэк,
    //!       а не вылетает std::terminate.
    const char* what() const noexcept override
    {
        if (!m_message.empty())
            return m_message.c_str();

        try
        {
            if (m_code != 0)
            {
                m_message = ensureTrailingPunctuation(
                    formatSystemErrorMessage(m_code));
            }
            else
            {
                m_message = "Unknown error";
            }
        }
        catch (...)
        {
            // Формирование не удалось (например, bad_alloc).
            // Возврат статического литерала не бросает и не нарушает noexcept.
            return "Unknown error (failed to format message)";
        }

        return m_message.c_str();
    }

    //------------------------------------------------------------------------
    //! Код системной ошибки (DWORD под Windows, int под *nix).
    ErrorCodeType code() const noexcept
    {
        return m_code;
    }

    //------------------------------------------------------------------------
    //! Установлен ли код ошибки.
    bool hasCode() const noexcept
    {
        return m_code != 0;
    }


protected:

    ErrorCodeType       m_code    = 0; //!< Код системной ошибки
    mutable std::string m_message   ;  //!< Кэш текстового описания (mutable из-за what())

}; // class SystemException

//----------------------------------------------------------------------------




//============================================================================
//
//  FileException — ошибка работы с файлом
//
//============================================================================

//----------------------------------------------------------------------------
/*!
    \brief Ошибка при работе с файлом.

    Хранит имя файла и, опционально, код системной ошибки. Код может быть
    нулевым, если ошибка имеет пользовательскую природу (например, ошибка
    разбора формата) — в этом случае what() вернёт только явно заданный
    текст, без системного описания.

    \note FileException НЕ является SystemException: не всякая файловая
          ошибка является системной. Если нужно ловить все ошибки с кодом ОС,
          ловите SystemException отдельно. Общий предок FileException и
          SystemException — umba::Exception.
*/
class FileException : public Exception
{
public:

    using ErrorCodeType = SystemErrorCode;

    //------------------------------------------------------------------------
    //! По умолчанию: код из GetLastError()/errno, без имени файла.
    FileException() noexcept = default;

    //------------------------------------------------------------------------
    //! Только имя файла — код берётся из текущей системной ошибки.
    UMBA_EXPLICIT
    FileException( std::string fileName )
        : m_code(getLastSystemError())
        , m_fileName(std::move(fileName))
    {}

    //------------------------------------------------------------------------
    //! Код ошибки + имя файла.
    FileException( ErrorCodeType code, std::string fileName )
        : m_code(code)
        , m_fileName(std::move(fileName))
    {}

    //------------------------------------------------------------------------
    //! Готовый текст + имя файла (без системного кода).
    FileException( std::string msg, std::string fileName )
        : m_code(0)
        , m_message(ensureTrailingPunctuation(std::move(msg)))
        , m_fileName(std::move(fileName))
    {}

    //------------------------------------------------------------------------
    //! Код + текст + имя файла.
    FileException( ErrorCodeType code, std::string msg, std::string fileName )
        : m_code(code)
        , m_message(ensureTrailingPunctuation(std::move(msg)))
        , m_fileName(std::move(fileName))
    {}

    //------------------------------------------------------------------------
    //! Текст ошибки. Формируется лениво по коду, если код задан и текст
    //! не передан явно.
    const char* what() const noexcept override
    {
        if (!m_message.empty())
            return m_message.c_str();

        try
        {
            if (m_code != 0)
            {
                m_message = ensureTrailingPunctuation(
                    formatSystemErrorMessage(m_code));
            }
            else
            {
                m_message = "Unknown file error";
            }
        }
        catch (...)
        {
            return "Unknown file error (failed to format message)";
        }

        return m_message.c_str();
    }

    //------------------------------------------------------------------------
    //! Имя файла.
    const char* getFileName() const noexcept
    {
        return m_fileName.c_str();
    }

    //------------------------------------------------------------------------
    //! Код системной ошибки, если он есть. 0 = ошибка не системная.
    ErrorCodeType code() const noexcept
    {
        return m_code;
    }

    //------------------------------------------------------------------------
    //! Был ли установлен код ОС.
    bool hasCode() const noexcept
    {
        return m_code != 0;
    }

protected:

    ErrorCodeType       m_code    = 0; //!< 0 = нет системного кода
    mutable std::string m_message   ;  //!< Кэш / пользовательский текст
    std::string         m_fileName  ;  //!< Имя файла

}; // class FileException

//----------------------------------------------------------------------------




//============================================================================
//
//  FileParsingException — ошибка разбора текстового файла
//
//============================================================================

//----------------------------------------------------------------------------
//! Ошибка разбора файла.
class FileParsingException : public FileException
{

public:

    //! Конструктор из текста сообщения, имени файла, номера строки и позиции
    //! в строке. Нумерация с нуля.
    UMBA_EXPLICIT
    FileParsingException( const std::string& msg
                        , const std::string& fileName
                        , size_t lineNumber = (size_t)-1
                        , size_t linePos    = (size_t)-1 )
        : FileException(msg, fileName)
        , m_lineNumber(lineNumber)
        , m_linePos(linePos)
    {}

    //! Конструктор из кода системной ошибки, текста, имени файла,
    //! номера строки и позиции в строке.
    FileParsingException( FileException::ErrorCodeType code
                        , const std::string& msg
                        , const std::string& fileName
                        , size_t lineNumber = (size_t)-1
                        , size_t linePos    = (size_t)-1 )
        : FileException(code, msg, fileName)
        , m_lineNumber(lineNumber)
        , m_linePos(linePos)
    {}

    //! Номер строки (нумерация с нуля, (size_t)-1 == «не задан»)
    size_t getLineNumber() const noexcept
    {
        return m_lineNumber;
    }

    //! Позиция в строке (нумерация с нуля, (size_t)-1 == «не задана»)
    size_t getLinePos() const noexcept
    {
        return m_linePos;
    }

protected:

    size_t m_lineNumber; //!< Номер строки
    size_t m_linePos;    //!< Позиция в строке

}; // class FileParsingException

//----------------------------------------------------------------------------




//============================================================================
//
//  FileBinaryParsingException — ошибка разбора двоичного файла
//
//============================================================================

//----------------------------------------------------------------------------
//! Ошибка разбора двоичного файла.
class FileBinaryParsingException : public FileException
{

public:

    //! Конструктор из текста, имени файла, смещения и готовой строки дампа.
    UMBA_EXPLICIT
    FileBinaryParsingException( const std::string& msg
                              , const std::string& fileName
                              , size_t offset
                              , const std::string &dump = std::string() )
        : FileException(msg, fileName)
        , m_offset(offset)
        , m_dump(dump)
    {}

    //! Конструктор из текста, имени файла, смещения и указателя на данные
    //! (для автоматического формирования дампа).
    UMBA_EXPLICIT
    FileBinaryParsingException( const std::string& msg
                              , const std::string& fileName
                              , size_t offset
                              , const uint8_t *pData
                              , size_t numBytes )
        : FileException(msg, fileName)
        , m_offset(offset)
        , m_dump(strGetDumpHelper(pData, numBytes))
    {}

    //! Деструктор
    //~FileBinaryParsingException() noexcept override = default;

    //! Смещение от начала файла.
    size_t getOffset() const noexcept
    {
        return m_offset;
    }

    //! Указатель на строку дампа.
    const char* getDump() const noexcept
    {
        return m_dump.c_str();
    }

    //! Задан ли дамп.
    bool hasDump() const noexcept
    {
        return !m_dump.empty();
    }

protected:

    //! Хелпер формирования строки с HEX-дампом.
    static
    std::string strGetDumpHelper( const uint8_t *pData, size_t numBytes )
    {
        if (!pData || numBytes==0)
            return std::string();

        auto toHexDigit = [](unsigned v) -> char
        {
            if (v < 10)
                return (char)(v + '0');
            if (v < 16)
                return (char)(v + 'A' - 10);
            return '?'; // не должно происходить
        };

        std::string resStr;
        resStr.reserve(numBytes*3);

        for (size_t i = 0; i != numBytes; ++i)
        {
            if (i)
                resStr.push_back(' ');
            const uint8_t b = pData[i];
            resStr.push_back(toHexDigit((b >> 4) & 0xFu));
            resStr.push_back(toHexDigit((b     ) & 0xFu));
        }

        return resStr;
    }

    size_t      m_offset = 0; //!< Смещение от начала файла
    std::string m_dump      ; //!< Дамп

}; // class FileBinaryParsingException

//----------------------------------------------------------------------------




//============================================================================
//
//  Настройка потока вывода
//
//============================================================================

//----------------------------------------------------------------------------
// Note - required to be
// using namespace ::umba::omanip
// or
// using namespace std::iomanip
// declared before using this macro

//----------------------------------------------------------------------------
//! Шаблон настройки потока вывода для вывода информации в HEX виде
template< typename StreamType >
inline
void exceptionHandlingConfigureStreamHex( StreamType & ss, bool setHex )
{
    // Bjarne, 740
    ss.setf( setHex ? std::ios_base::hex : std::ios_base::dec, std::ios_base::basefield );
}

//----------------------------------------------------------------------------
#ifdef UMBA_SIMPLE_FORMATTER_H
//! Специализация шаблона настройки потока вывода для вывода информации в HEX виде для потока umba::SimpleFormatter
template< >
inline
void exceptionHandlingConfigureStreamHex<::umba::SimpleFormatter>( ::umba::SimpleFormatter & ss, bool setHex )
{
    ss.base( setHex ? 16 : 10 );
}
#endif

//----------------------------------------------------------------------------
//! Шаблон настройки потока вывода для вывода информации с расцветкой
template< typename StreamType >
inline
void exceptionHandlingConfigureStreamColoringError( StreamType & /*ss*/ )
{
}

//----------------------------------------------------------------------------
#ifdef UMBA_SIMPLE_FORMATTER_H
//! Специализация шаблона настройки потока вывода для вывода информации с расцветкой для потока umba::SimpleFormatter
template< >
inline
void exceptionHandlingConfigureStreamColoringError<::umba::SimpleFormatter>( ::umba::SimpleFormatter & ss )
{
    using namespace ::umba::omanip;
    //ss<<error<<"error:";
    //ss<<normal<<" ";

    ss<<error;
}
#endif

//----------------------------------------------------------------------------
//! Шаблон для вывода префикса ошибки
template< typename StreamType >
inline
void exceptionHandlingConfigureStreamColoringErrorMessage( StreamType & /*ss*/ )
{
}

//----------------------------------------------------------------------------
#ifdef UMBA_SIMPLE_FORMATTER_H
//! Специализация шаблона для вывода префикса ошибки для потока umba::SimpleFormatter
template< >
inline
void exceptionHandlingConfigureStreamColoringErrorMessage<::umba::SimpleFormatter>( ::umba::SimpleFormatter & ss )
{
    using namespace ::umba::omanip;
    ss<<error;
    ss<<"error:";
    ss<<normal<<" ";
}
#endif

//----------------------------------------------------------------------------




//============================================================================
//
//  Печать исключений
//
//============================================================================

//----------------------------------------------------------------------------
//! Шаблон вывода FileParsingException в поток вывода
template< typename Stream >
inline
void printException( Stream &stream, const ::umba::FileParsingException &e )
{
    //using namespace ::umba::omanip;

    exceptionHandlingConfigureStreamColoringError(stream);

    const size_t lineNumber = e.getLineNumber();
    const size_t linePos    = e.getLinePos();

    if (lineNumber == (size_t)-1)
    {
        stream<<e.getFileName()<<": ";
        //<<e.what()<<"\n";
    }
    else if (linePos == (size_t)-1)
    {
        stream<<e.getFileName()<<":"<<(lineNumber+1)<<": ";
    }
    else
    {
        stream<<e.getFileName()<<":"<<(lineNumber+1)<<":"<<(linePos+1)<<": ";
    }

    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<e.what()<<"\n";
}

//----------------------------------------------------------------------------
//! Шаблон вывода FileBinaryParsingException в поток вывода
template< typename Stream >
inline
void printException( Stream &stream, const ::umba::FileBinaryParsingException &e )
{
    //using namespace ::umba::omanip;

    exceptionHandlingConfigureStreamColoringError(stream);
    stream<<e.getFileName()<<": offset "<<e.getOffset()<<" (";
    exceptionHandlingConfigureStreamHex( stream, true );
    stream<<e.getOffset();
    exceptionHandlingConfigureStreamHex( stream, false );
    stream<<")"<<": ";

    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<e.what();
    if (e.hasDump())
        stream<<". Dump: "<<e.getDump();
    stream<<"\n";
}

//----------------------------------------------------------------------------
//! Шаблон вывода FileException в поток вывода
template< typename Stream >
inline
void printException( Stream &stream, const ::umba::FileException &e )
{
    //using namespace ::umba::omanip;
    exceptionHandlingConfigureStreamColoringError(stream);
    stream<<e.getFileName()<<": ";
    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<e.what()<<"\n";
}

//----------------------------------------------------------------------------
//! Шаблон вывода SystemException в поток вывода
template< typename Stream >
inline
void printException( Stream &stream, const ::umba::SystemException &e )
{
    exceptionHandlingConfigureStreamColoringError(stream);
    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<e.what()<<"\n";
}

//----------------------------------------------------------------------------
//! Шаблон вывода любого umba-исключения в поток вывода.
//! Используется, когда конкретный тип исключения неизвестен, но точно
//! относится к библиотеке umba.
template< typename Stream >
inline
void printException( Stream &stream, const ::umba::Exception &e )
{
    exceptionHandlingConfigureStreamColoringError(stream);
    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<e.what()<<"\n";
}

//----------------------------------------------------------------------------
//! Шаблон вывода std::exception в поток вывода
template< typename Stream >
inline
void printException( Stream &stream, const std::exception &e )
{
    //using namespace ::umba::omanip;
    exceptionHandlingConfigureStreamColoringError(stream);
    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<e.what()<<"\n";
}

//----------------------------------------------------------------------------
//! Шаблон вывода информации "неизвестная ошибка" в поток вывода
template< typename Stream >
inline
void printUnknownException( Stream &stream )
{
    exceptionHandlingConfigureStreamColoringError(stream);
    exceptionHandlingConfigureStreamColoringErrorMessage(stream);
    stream<<"Unknown error\n";
}

//----------------------------------------------------------------------------

















//----------------------------------------------------------------------------
//! Макрос выкидывания исключения FileParsingException \ingroup UMBA_INTERNALS
#define UMBA_EXCEPTION_THROW_FILEPARSINGEXCEPTION( fileInfoGetFrom ,msg ) \
             throw ::umba::FileParsingException( msg, fileInfoGetFrom.getFileName(), fileInfoGetFrom.getLineNumber() )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Макрос печати исключения в поток \ingroup UMBA_INTERNALS
#define UMBA_EXCEPTION_TO_STREAM_PRINT( stream, e )            ::umba::printException(stream, e)
//! Макрос печати исключения в поток с последующим переводом строки \ingroup UMBA_INTERNALS
#define UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e )          do{ ::umba::printException(stream, e); stream<<"\n"; } while(0)

//! Макрос печати "неизвестная ошибка" в поток \ingroup UMBA_INTERNALS
#define UMBA_UNKNOWN_EXCEPTION_TO_STREAM_PRINT( stream )       ::umba::printUnknownException(stream)
//! Макрос печати "неизвестная ошибка" в поток с последующим переводом строки \ingroup UMBA_INTERNALS
#define UMBA_UNKNOWN_EXCEPTION_TO_STREAM_PRINTLN( stream )     do{ ::umba::printUnknownException(stream); stream<<"\n"; } while(0)

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Макрос ловли исключений с выводом в обычный поток.
//!
//! Порядок catch-блоков важен: производные — раньше базовых.
//! FileException и SystemException — независимые ветви, порядок между
//! ними не важен, но оба должны идти до std::exception.
//----------------------------------------------------------------------------
#define UMBA_EXCEPTION_CATCH_LOG_TO_STREAM( errFlag, stream )                          \
                                                                                       \
    catch( const ::umba::FileBinaryParsingException &e )                               \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::FileParsingException &e )                                     \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::FileException &e )                                            \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::SystemException &e )                                          \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::Exception &e )                                                \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const std::exception &e )                                                   \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( ... )                                                                       \
    {                                                                                  \
        UMBA_UNKNOWN_EXCEPTION_TO_STREAM_PRINTLN(stream);                              \
        errFlag = true;                                                                \
    }

//----------------------------------------------------------------------------
//! Макрос ловли исключений с выводом в поток umba::SimpleFormatter
#define UMBA_EXCEPTION_CATCH_LOG_TO_UMBA_STREAM( errFlag, stream )                     \
                                                                                       \
    catch( const ::umba::FileBinaryParsingException &e )                               \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::FileParsingException &e )                                     \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::FileException &e )                                            \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::SystemException &e )                                          \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const ::umba::Exception &e )                                                \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( const std::exception &e )                                                   \
    {                                                                                  \
        UMBA_EXCEPTION_TO_STREAM_PRINTLN( stream, e );                                 \
        errFlag = true;                                                                \
    }                                                                                  \
    catch( ... )                                                                       \
    {                                                                                  \
        UMBA_UNKNOWN_EXCEPTION_TO_STREAM_PRINTLN(stream);                              \
        errFlag = true;                                                                \
    }



} // namespace umba

