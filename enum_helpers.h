/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief Хелперы для enum'ов

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

//-----------------------------------------------------------------------------

#include "preprocessor.h"
#include "stl.h"
#include "umba.h"

#include <cstddef>
#include <cstring>

#if !defined(UMBA_MCU_USED)
    #include <map>
    #include <string>
    #include <vector>
#endif

#if defined(UMBA_QT_USED)
    #include <QString>
    #include <QStringList>
#endif


// umba::enum_helpers::



// umba::enum_helpers::toUnderlyingType
// umba::enum_helpers::fromUnderlyingType


//----------------------------------------------------------------------------
namespace umba {
namespace enum_helpers {



//----------------------------------------------------------------------------
//! Конвертирует enum в подлежащий тип. Версия для 'честных' enum'ов.
template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value
                                                    , bool
                                                    >::type = true
        > inline
typename std::underlying_type< EnumType >::type toUnderlyingType( EnumType flagsVal )
{
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    return (EnumUnderlyingType)flagsVal;
}

//----------------------------------------------------------------------------
//! Конвертирует enum в подлежащий тип. Версия для интегральных типов.
/*! Часто неохота разбираться, является ли значение int'ом, unsigned'ом, или другим интегральным типом,
    или же является enum'ом.
 */
template< typename EnumType, typename std::enable_if< (!std::is_enum<EnumType>::value
                                                    &&  std::is_integral<EnumType>::value
                                                      )
                                                    , bool
                                                    >::type = true
        > inline
EnumType toUnderlyingType( EnumType flagsVal )
{
    return flagsVal;
}

//----------------------------------------------------------------------------
//! Конвертирует в enum из подлежащего типа. Версия для 'честных' enum'ов.
template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, bool
                                                    >::type = true > inline
EnumType fromUnderlyingType( typename std::underlying_type< EnumType >::type flagsVal )
{
    return (EnumType)flagsVal;
}

//----------------------------------------------------------------------------
//! Конвертирует в enum из подлежащего типа. Версия для интегральных типов.
/*! Часто неохота разбираться, является ли значение int'ом, unsigned'ом, или другим интегральным типом,
    или же является enum'ом.
 */
template< typename EnumType, typename std::enable_if< (!std::is_enum<EnumType>::value 
                                                    &&  std::is_integral< EnumType >::value
                                                      )
                                                    , bool>::type = true 
                                                    > inline
EnumType fromUnderlyingType( EnumType flagsVal )
{
    return flagsVal;
}

//----------------------------------------------------------------------------
template< typename EnumType > inline
bool enumLessImpl(EnumType e1, EnumType e2)
{
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    return (EnumUnderlyingType)e1 < (EnumUnderlyingType)e2;
}

//----------------------------------------------------------------------------
template< typename EnumType > inline
bool enumLessEqualImpl(EnumType e1, EnumType e2)
{
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    return (EnumUnderlyingType)e1 <= (EnumUnderlyingType)e2;
}

//----------------------------------------------------------------------------
template< typename EnumType > inline
bool enumGreaterImpl(EnumType e1, EnumType e2)
{
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    return (EnumUnderlyingType)e1 > (EnumUnderlyingType)e2;
}

//----------------------------------------------------------------------------
template< typename EnumType > inline
bool enumGreaterEqualImpl(EnumType e1, EnumType e2)
{
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    return (EnumUnderlyingType)e1 >= (EnumUnderlyingType)e2;
}


} // namespace enum_helpers
} // namespace umba

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------
#define UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNARY_OPERATOR_IMPL(EnumType, operatorSign)                              \
            inline                                                                                              \
            EnumType operator operatorSign( EnumType e )                                                        \
            {                                                                                                   \
                return umba::enum_helpers::fromUnderlyingType<EnumType>(                                        \
                                           operatorSign                                                         \
                                           umba::enum_helpers::toUnderlyingType<EnumType>(e)                    \
                );                                                                                              \
            }

//------------------------------
#define UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType, operatorSign)                             \
            inline                                                                                              \
            EnumType operator operatorSign( EnumType e1, EnumType e2)                                           \
            {                                                                                                   \
                return umba::enum_helpers::fromUnderlyingType<EnumType>(                                        \
                                           umba::enum_helpers::toUnderlyingType<EnumType>(e1)                   \
                                           operatorSign                                                         \
                                           umba::enum_helpers::toUnderlyingType<EnumType>(e2)                   \
                );                                                                                              \
            }                                                                                                   \
            inline                                                                                              \
            EnumType& operator operatorSign##=( EnumType &e1, EnumType e2)                                      \
            {                                                                                                   \
                e1 = e1 operatorSign e2;                                                                        \
                return e1;                                                                                      \
            }

//------------------------------
#define UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType, operatorSign)             \
            inline                                                                                              \
            EnumType operator operatorSign( EnumType e1, typename std::underlying_type< EnumType >::type e2)    \
            {                                                                                                   \
                return umba::enum_helpers::fromUnderlyingType<EnumType>(                                        \
                                           umba::enum_helpers::toUnderlyingType<EnumType>(e1)                   \
                                           operatorSign                                                         \
                                           e2                                                                   \
                );                                                                                              \
            }                                                                                                   \
            inline                                                                                              \
            EnumType operator operatorSign( typename std::underlying_type< EnumType >::type e1, EnumType e2)    \
            {                                                                                                   \
                return umba::enum_helpers::fromUnderlyingType<EnumType>(                                        \
                                           e1                                                                   \
                                           operatorSign                                                         \
                                           umba::enum_helpers::toUnderlyingType<EnumType>(e2)                   \
                );                                                                                              \
            }                                                                                                   \
            inline                                                                                              \
            EnumType& operator operatorSign##=( EnumType &e1, typename std::underlying_type< EnumType >::type e2) \
            {                                                                                                   \
                e1 = e1 operatorSign e2;                                                                        \
                return e1;                                                                                      \
            }

//------------------------------
#define UMBA_ENUM_CLASS_IMPLEMENT_ENUM_SHIFT_OPERATOR_IMPL(EnumType, operatorSign)                              \
            inline                                                                                              \
            EnumType operator operatorSign( EnumType e, unsigned sh )                                           \
            {                                                                                                   \
                return umba::enum_helpers::fromUnderlyingType<EnumType>(                                        \
                                           umba::enum_helpers::toUnderlyingType<EnumType>(e)                    \
                                           operatorSign                                                         \
                                           sh                                                                   \
                );                                                                                              \
            }                                                                                                   \
            inline                                                                                              \
            EnumType& operator operatorSign##=( EnumType &e, unsigned sh )                                      \
            {                                                                                                   \
                e = e operatorSign sh;                                                                          \
                return e;                                                                                       \
            }

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Реализует битовые операции для enum-типа
#define UMBA_ENUM_CLASS_IMPLEMENT_BIT_OPERATORS( EnumType )                                                     \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNARY_OPERATOR_IMPL (EnumType,~)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,|)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,&)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,^)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_SHIFT_OPERATOR_IMPL (EnumType,<<)                                   \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_SHIFT_OPERATOR_IMPL (EnumType,>>)


//----------------------------------------------------------------------------
//! Реализует битовые операции для enum-типа и подлежащего типа
#define UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_BIT_OPERATORS( EnumType )                                     \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,|)                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,&)                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,^)


//----------------------------------------------------------------------------
//! Реализует арифметические операции для enum-типа
#define UMBA_ENUM_CLASS_IMPLEMENT_ARITHMETIC_OPERATORS( EnumType )                                              \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNARY_OPERATOR_IMPL (EnumType,+)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNARY_OPERATOR_IMPL (EnumType,-)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,+)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,-)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,*)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,%)                                    \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_BINARY_OPERATOR_IMPL(EnumType,/)


//----------------------------------------------------------------------------
//! Реализует арифметические операции для enum-типа и подлежащего типа
#define UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_ARITHMETIC_OPERATORS( EnumType )                               \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,+)                     \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,-)                     \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,*)                     \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,/)                     \
             UMBA_ENUM_CLASS_IMPLEMENT_ENUM_UNDERLYING_TYPE_BINARY_OPERATOR_IMPL(EnumType,%)


//----------------------------------------------------------------------------
//! Реализует операции сравнения больше/меньше для enum-типа
#define UMBA_ENUM_CLASS_IMPLEMENT_RELATION_OPERATORS( EnumType )                                                            \
                                                                                                                            \
             inline bool operator< (EnumType e1, EnumType e2) { return umba::enum_helpers::enumLessImpl        (e1, e2); }  \
             inline bool operator<=(EnumType e1, EnumType e2) { return umba::enum_helpers::enumLessEqualImpl   (e1, e2); }  \
             inline bool operator> (EnumType e1, EnumType e2) { return umba::enum_helpers::enumGreaterImpl     (e1, e2); }  \
             inline bool operator>=(EnumType e1, EnumType e2) { return umba::enum_helpers::enumGreaterEqualImpl(e1, e2); }


//----------------------------------------------------------------------------
//! Реализует операции сравнения больше/меньше для enum-типа и подлежащего типа
#define UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_RELATION_OPERATORS( EnumType )                                                                                            \
                                                                                                                                                                            \
             inline bool operator< (EnumType e1, typename std::underlying_type< EnumType >::type e2) { return umba::enum_helpers::enumLessImpl        (e1, (EnumType)e2); } \
             inline bool operator<=(EnumType e1, typename std::underlying_type< EnumType >::type e2) { return umba::enum_helpers::enumLessEqualImpl   (e1, (EnumType)e2); } \
             inline bool operator> (EnumType e1, typename std::underlying_type< EnumType >::type e2) { return umba::enum_helpers::enumGreaterImpl     (e1, (EnumType)e2); } \
             inline bool operator>=(EnumType e1, typename std::underlying_type< EnumType >::type e2) { return umba::enum_helpers::enumGreaterEqualImpl(e1, (EnumType)e2); } \
                                                                                                                                                                            \
             inline bool operator< (typename std::underlying_type< EnumType >::type e1, EnumType e2) { return umba::enum_helpers::enumLessImpl        ((EnumType)e1, e2); } \
             inline bool operator<=(typename std::underlying_type< EnumType >::type e1, EnumType e2) { return umba::enum_helpers::enumLessEqualImpl   ((EnumType)e1, e2); } \
             inline bool operator> (typename std::underlying_type< EnumType >::type e1, EnumType e2) { return umba::enum_helpers::enumGreaterImpl     ((EnumType)e1, e2); } \
             inline bool operator>=(typename std::underlying_type< EnumType >::type e1, EnumType e2) { return umba::enum_helpers::enumGreaterEqualImpl((EnumType)e1, e2); }



//----------------------------------------------------------------------------
#define UMBA_ENUM_CLASS_IMPLEMENT_UNDERLYING_TYPE_EQUAL_OPERATORS( EnumType )                                                   \
                                                                                                                                \
             template<typename IntType, typename std::enable_if<std::is_integral<IntType>::value, bool>::type = true > inline   \
             bool operator==(EnumType e, IntType i)                                                                             \
             {                                                                                                                  \
                 using TUnder = typename std::underlying_type<EnumType>::type;                                                  \
                 return static_cast<TUnder>(e)==static_cast<TUnder>(i);                                                         \
             }                                                                                                                  \
                                                                                                                                \
             template<typename IntType, typename std::enable_if<std::is_integral<IntType>::value, bool>::type = true > inline   \
             bool operator==(IntType i, EnumType e)                                                                             \
             {                                                                                                                  \
                 using TUnder = typename std::underlying_type<EnumType>::type;                                                  \
                 return static_cast<TUnder>(i)==static_cast<TUnder>(e);                                                         \
             }                                                                                                                  \
                                                                                                                                \
             template<typename IntType, typename std::enable_if<std::is_integral<IntType>::value, bool>::type = true > inline   \
             bool operator!=(EnumType e, IntType i)                                                                             \
             {                                                                                                                  \
                 using TUnder = typename std::underlying_type<EnumType>::type;                                                  \
                 return static_cast<TUnder>(e)!=static_cast<TUnder>(i);                                                         \
             }                                                                                                                  \
                                                                                                                                \
             template<typename IntType, typename std::enable_if<std::is_integral<IntType>::value, bool>::type = true > inline   \
             bool operator!=(IntType i, EnumType e)                                                                             \
             {                                                                                                                  \
                 using TUnder = typename std::underlying_type<EnumType>::type;                                                  \
                 return static_cast<TUnder>(i)!=static_cast<TUnder>(e);                                                         \
             }                                                                                                                  \
                                                                                                                                \
             inline                                                                                                             \
             bool operator!(EnumType e)                                                                                         \
             {                                                                                                                  \
                 using TUnder = typename std::underlying_type<EnumType>::type;                                                  \
                 return static_cast<TUnder>(e)==0;                                                                              \
             }

/*
             template<typename EnumType, typename IntType> inline                                               \
             bool operator==(IntType i, EnumType e)                                                             \
             {                                                                                                  \
                 return umba::enum_helpers::toUnderlyingType(e)==umba::enum_helpers::toUnderlyingType(i);       \
             }                                                                                                  \
                                                                                                                \
             template<typename EnumType, typename IntType> inline                                               \
             bool operator!=(IntType i, EnumType e)                                                             \
             {                                                                                                  \
                 return umba::enum_helpers::toUnderlyingType(e)!=umba::enum_helpers::toUnderlyingType(i);       \
             }                                                                                                  \

*/



//----------------------------------------------------------------------------






#if 0

inline                                                                                                          \
bool operator==(EnumType e, std::underlying_type< EnumType >::type i)                                           \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return i==(EnumUnderlyingType)e;                                                                            \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
bool operator==(std::underlying_type< EnumType >::type i, EnumType e)                                           \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return i==(EnumUnderlyingType)e;                                                                            \
}
#endif

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------
namespace umba
{



//----------------------------------------------------------------------------
#if !defined(UMBA_MCU_USED)

//! Генерирует вектор последовательных значений enum'а
template< typename EnumType >
inline
std::vector<EnumType> enumValuesToVector( EnumType eb                 //!< Первое значение
                                        , EnumType ee                 //!< Последнее значение
                                        , bool inclusiveEnd = false   //!< Включать ли последнее значение. Если false (default), то eb/ee работают как begin()/end() контейнеров std
                                        )
{
    // size_t b = (size_t)eb;
    // size_t e = (size_t)ee;

    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    EnumUnderlyingType b = (EnumUnderlyingType)eb;
    EnumUnderlyingType e = (EnumUnderlyingType)ee;
    if (inclusiveEnd)
        ++e;

    std::vector<EnumType> res;
    res.reserve( e-b );

    for( ; b!=e; ++b)
        res.push_back( (EnumType)b );

    return res;

}

// Для однообразия старое кладём в новый неймспейс
namespace enum_helpers {

template< typename EnumType >
inline
std::vector<EnumType> enumValuesToVector( EnumType eb, EnumType ee, bool inclusiveEnd = false )
{
    return umba::enumValuesToVector( eb, ee, inclusiveEnd );
}

} // namespace enum_helpers


#endif // !UMBA_MCU_USED






//----------------------------------------------------------------------------

} // namespace umba

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

namespace umba {
namespace enum_helpers {




//----------------------------------------------------------------------------
#if !defined(UMBA_MCU_USED)

    //! Нормализация имени enum'а (std::string версия) - строковые значения enum'ов не зависят от регистра
    inline
    std::string toNormalEnumNameCase( const std::string &s )
    {
        std::string res; res.reserve(s.size());

        std::string::size_type i = 0, sz = s.size();

        for(; i!=sz; ++i )
        {
            char ch = s[i];
            if (ch>='a' && ch<='z')
               ch = ch - 'a' + 'A';
            res.append(1,ch);
        }

        return res;
    }

    //! Перегрузка для сравнения строк-значений enum'а
    inline bool isStringsEqual( const std::string &s1, const std::string &s2 )
    {
        return s1==s2;
    }

    //! Возвращает главное строковое представление для перечисления из строки со всеми вариантами. Оно используется для сериализации.
    /*! При десериализации может быть несколько вариантов, при сериализации - только один
     */
    inline std::string toEnumName( const std::string &s )
    {
        //TODO: !!! Сделать так же, как для QString
        return s;
    }

    //! Создаёт std::map с ключами - именами enum'а
    template< typename EnumType > inline
    void fillMapForEnumEntry( std::map< std::string, EnumType> &m, EnumType v, const std::string &strName )
    {
        //TODO: !!! Сделать так же, как для QString
        m[toNormalEnumNameCase(strName)] = v;
    }

    //! Создаёт std::map с ключами - именами enum'а - перегрузка для строковых констант
    template< typename EnumType > inline
    void fillMapForEnumEntry( std::map< std::string, EnumType> &m, EnumType v, const char *strName )
    {
        fillMapForEnumEntry( m, v, std::string(strName) );
    }

#endif // !defined(UMBA_MCU_USED)

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#if defined(UMBA_QT_USED)

    //! Нормализация имени enum'а (QString версия) - строковые значения enum'ов не зависят от регистра
    inline QString toNormalEnumNameCase( const QString &s )              { return s.toUpper(); }

    //! Перегрузка для сравнения строк-значений enum'а
    inline bool isStringsEqual( const QString &s1, const QString &s2 )   { return s1==s2; }

    //! Возвращает главное строковое представление для перечисления из строки со всеми вариантами. Оно используется для сериализации.
    /*! При десериализации может быть несколько вариантов, при сериализации - только один
     */
    inline QString toEnumName( const QString &s )
    {
        QStringList names = s.split(QChar('|'), Qt::KeepEmptyParts);
        if (names.empty())
            return QString();
        return names[0];
    }

    //! Создаёт std::map с ключами - именами enum'а
    template< typename EnumType > inline
    void fillMapForEnumEntry( std::map< QString, EnumType> &m, EnumType v, const QString &strName )
    {
        QStringList names = strName.split(QChar('|'), Qt::KeepEmptyParts);
        for( auto n : names )
        {
            m[toNormalEnumNameCase(n)] = v;
        }
    }

    //! Создаёт std::map с ключами - именами enum'а - перегрузка для строковых констант
    template< typename EnumType > inline
    void fillMapForEnumEntry( std::map< QString, EnumType> &m, EnumType v, const char *strName )
    {
        fillMapForEnumEntry( m, v, QString::fromLatin1(strName) );
    }

    inline
    QStringList makeEnumValuesStringList( const char** names, std::size_t sz )
    {
        QStringList res;

        for(std::size_t i=0; i!=sz; ++i)
            res << names[i];

        return res;
    }



#endif // defined(UMBA_QT_USED)

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Нормализация имени enum'а (QString версия) - строковые значения enum'ов не зависят от регистра
/*! Всеплатформенная версия, годная и для MCU. Регистр строкового значения enum'а остаётся прежним
 */
inline
const char* toNormalEnumNameCase( const char* s )
{
    return s;
}

//! Перегрузка для сравнения строк-значений enum'а
inline
bool isStringsEqual( const char* s1, const char* s2 )
{
    return std::strcmp(s1,s2)==0;
}

//! Возвращает главное строковое представление для перечисления из строки со всеми вариантами. Оно используется для сериализации.
/*! Всеплатформенная версия, годная и для MCU. Для MCU не поддерживается множественное представление одной величины, поэтому вход возвращается как есть.
 */
inline
const char* toEnumName( const char *s )
{
    return s;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
} // namespace enum_helpers
} // namespace umba

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
// Enum requres members: 'invalid', 'begin', 'end'
// QString version allows multiple string representations for single value




//----------------------------------------------------------------------------
//! Генерирует функцию-хелпер для преобразования значения enum'а в строку. Реализация.
/*!
    storageDuration - static / inline
    stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString

    Генерирует функции

       const char** fnNamePrefixTo##stringTypeName##GetNamesImpl( std::size_t *pCount ) - Возвращает указатель на массив имён и его размер (через OUT параметр)
       stringType   fnNamePrefix##To##stringTypeName##ImplHelper( enumType t )          - Хелпер, возвращает имя по значению enum'а (вернее, не имя, а строку с альтернативами, перечисленными через символ '|')
       stringType   fnNamePrefix##To##stringTypeName( enumType t )                      - Возвращает возвращает имя по значению enum'а (первое из списка альтернатив)
       const char** fnNamePrefix##To##stringTypeName##GetNames()                        - Возвращает указатель на массив имён
       std::size_t  fnNamePrefix##To##stringTypeName##GetNamesNumber()                  - Возвращает размер массива имён
       stringType   fnNamePrefix##_##to##stringTypeName( enumType t )                   - Возвращает возвращает имя по значению enum'а (первое из списка альтернатив) - версия с подчёркиванием после имени enum'а
       const char** fnNamePrefix##_##to##stringTypeName##GetNames( )                    - Возвращает указатель на массив имён - версия с подчёркиванием после имени enum'а
       std::size_t  fnNamePrefix##_##to##stringTypeName##GetNamesNumber( )              - Возвращает размер массива имён - версия с подчёркиванием после имени enum'а



 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_IMPL( storageDuration, stringType, enumType, fnNamePrefix, stringTypeName, ... ) \
                                                                                                              \
                storageDuration                                                                               \
                const char** fnNamePrefix##To##stringTypeName##GetNamesImpl( std::size_t *pCount = 0)         \
                {                                                                                             \
                    static const char* names[] = { __VA_ARGS__ };                                             \
                    if (pCount)                                                                               \
                        *pCount = UMBA_COUNT_OF(names);                                                       \
                    return &names[0];                                                                         \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                stringType fnNamePrefix##To##stringTypeName##ImplHelper( enumType t )                         \
                {                                                                                             \
                    typedef std::underlying_type< enumType >::type  EnumUnderlyingType;                       \
                    EnumUnderlyingType ut = static_cast<EnumUnderlyingType>(t);                               \
                    EnumUnderlyingType ub = static_cast<EnumUnderlyingType>(enumType::begin);                 \
                                                                                                              \
                    std::size_t idx = (std::size_t)(ut - ub);                                                 \
                                                                                                              \
                    std::size_t namesNumber = 0;                                                              \
                    const char** names = fnNamePrefix##To##stringTypeName##GetNamesImpl(&namesNumber);        \
                                                                                                              \
                    if (idx>=namesNumber) return (stringType)("invalid");                                     \
                                                                                                              \
                    return (stringType)(names[idx]);                                                          \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                stringType fnNamePrefix##To##stringTypeName( enumType t )                                     \
                {                                                                                             \
                    return umba::enum_helpers::toEnumName(fnNamePrefix##To##stringTypeName##ImplHelper(t));   \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                const char** fnNamePrefix##To##stringTypeName##GetNames( )                                    \
                {                                                                                             \
                    return fnNamePrefix##To##stringTypeName##GetNamesImpl(0);                                 \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                std::size_t fnNamePrefix##To##stringTypeName##GetNamesNumber( )                               \
                {                                                                                             \
                    std::size_t num = 0;                                                                      \
                    fnNamePrefix##To##stringTypeName##GetNamesImpl(&num);                                     \
                    return num;                                                                               \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                stringType fnNamePrefix##_##to##stringTypeName( enumType t )                                    \
                {                                                                                             \
                    return fnNamePrefix##To##stringTypeName( t );                                             \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                const char** fnNamePrefix##_##to##stringTypeName##GetNames( )                                   \
                {                                                                                             \
                    return fnNamePrefix##To##stringTypeName##GetNamesImpl(0);                                 \
                }                                                                                             \
                                                                                                              \
                storageDuration                                                                               \
                std::size_t fnNamePrefix##_##to##stringTypeName##GetNamesNumber( )                              \
                {                                                                                             \
                    std::size_t num = 0;                                                                      \
                    fnNamePrefix##To##stringTypeName##GetNamesImpl(&num);                                     \
                    return num;                                                                               \
                }

//----------------------------------------------------------------------------
// typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
//! Генерирует функцию-хелпер для преобразования значения enum'а в строку. Версия для свободных enum'ов.
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING( stringType, enumType, fnNamePrefix, stringTypeName, ... )                    \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_IMPL( inline, stringType, enumType, fnNamePrefix, stringTypeName, __VA_ARGS__ )


//----------------------------------------------------------------------------
// typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
//! Генерирует функцию-хелпер для преобразования значения enum'а в строку. Версия для использования в классе (когда enum объявлен в классе).
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName, ... )             \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_IMPL( static, stringType, enumType, fnNamePrefix, stringTypeName, __VA_ARGS__ )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Генерирует функцию, возвращающую std::map, где ключи - строковые представления enum'а. Реализация.
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP_IMPL( storageDuration, stringType, enumType, fnNamePrefix, stringTypeName ) \
                                                                                                       \
storageDuration                                                                                        \
const std::map<stringType,enumType>& fnNamePrefix##To##stringTypeName##GetMap()                        \
{                                                                                                      \
    static std::map<stringType,enumType> m;                                                            \
                                                                                                       \
    if (!m.empty()) return m;                                                                          \
                                                                                                       \
    typedef std::underlying_type< enumType >::type  EnumUnderlyingType;                                \
    EnumUnderlyingType ub = static_cast<EnumUnderlyingType>(enumType::begin);                          \
    EnumUnderlyingType ue = static_cast<EnumUnderlyingType>(enumType::end  );                          \
                                                                                                       \
    for(; ub!=ue; ++ub)                                                                                \
       umba::enum_helpers::fillMapForEnumEntry( m, (enumType)ub, fnNamePrefix##To##stringTypeName##ImplHelper((enumType)ub) ); \
                                                                                                       \
    return m;                                                                                          \
}

//----------------------------------------------------------------------------
//! Генерирует функцию, возвращающую std::map, где ключи - строковые представления enum'а. Версия для свободных enum'ов.
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP( stringType, enumType, fnNamePrefix, stringTypeName )            \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP_IMPL( inline, stringType, enumType, fnNamePrefix, stringTypeName )

//----------------------------------------------------------------------------

//! Генерирует функцию, возвращающую std::map, где ключи - строковые представления enum'а. Версия для использования в классе (когда enum объявлен в классе).
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName )     \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP_IMPL( static, stringType, enumType, fnNamePrefix, stringTypeName )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Конвертация из строки в enum с использованием std::map. Реализация.
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP_IMPL( storageDuration, stringType, enumType, fnNamePrefix, stringTypeName ) \
                                                                                                       \
storageDuration                                                                                        \
enumType fnNamePrefix##From##stringTypeName( stringType t )                                            \
{                                                                                                      \
    t = umba::enum_helpers::toNormalEnumNameCase(t);                                                   \
    const std::map<stringType,enumType>& m = fnNamePrefix##To##stringTypeName##GetMap();               \
    std::map<stringType,enumType>::const_iterator it = m.find( t );                                    \
    if (it!=m.end()) return it->second;                                                                \
    return enumType :: invalid;                                                                        \
}                                                                                                      \
                                                                                                       \
storageDuration                                                                                        \
enumType fnNamePrefix##_##from##stringTypeName( stringType t )                                           \
{                                                                                                      \
    return fnNamePrefix##From##stringTypeName( t );                                                    \
}

//----------------------------------------------------------------------------

//! Конвертация из строки в enum с использованием std::map. Версия для свободных enum'ов.
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP( stringType, enumType, fnNamePrefix, stringTypeName )            \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP_IMPL( inline, stringType, enumType, fnNamePrefix, stringTypeName )


//----------------------------------------------------------------------------

//! Конвертация из строки в enum с использованием std::map. Версия для использования в классе (когда enum объявлен в классе).
/*! stringType      - тип строки - std::string, QString, const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: std::string -> StdString
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName )     \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP_IMPL( static, stringType, enumType, fnNamePrefix, stringTypeName )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Конвертация из строки в enum с использованием перебора. Реализация для MCU.
/*! stringType      - тип строки - const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: ConstCharPtr
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU_IMPL( storageDuration, stringType, enumType, fnNamePrefix, stringTypeName ) \
                                                                                                       \
storageDuration                                                                                        \
enumType fnNamePrefix##From##stringTypeName( stringType t )                                            \
{                                                                                                      \
    t = umba::enum_helpers::toNormalEnumNameCase(t);                                                   \
                                                                                                       \
    unsigned ub = (unsigned) enumType :: begin;                                                        \
    unsigned ue = (unsigned) enumType :: end;                                                          \
                                                                                                       \
    for(; ub!=ue; ++ub)                                                                                \
    {                                                                                                  \
        if (isStringsEqual( fnNamePrefix##To##stringTypeName((enumType)ub), t ))                       \
            return (enumType)ub;                                                                       \
    }                                                                                                  \
    return enumType :: invalid;                                                                        \
}                                                                                                      \
                                                                                                       \
storageDuration                                                                                        \
enumType fnNamePrefix##_##From##stringTypeName( stringType t )                                           \
{                                                                                                      \
    return fnNamePrefix##From##stringTypeName( t );                                                    \
}

//----------------------------------------------------------------------------
//! Конвертация из строки в enum с использованием перебора. Реализация для MCU. Версия для свободных enum'ов.
/*! stringType      - тип строки - const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: ConstCharPtr
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU( stringType, enumType, fnNamePrefix, stringTypeName )            \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU_IMPL( inline, stringType, enumType, fnNamePrefix, stringTypeName )

//----------------------------------------------------------------------------
//! Конвертация из строки в enum с использованием перебора. Реализация для MCU. Версия для использования в классе (когда enum объявлен в классе).
/*! stringType      - тип строки - const char*, ...
    enumType        - тип enum'а
    fnNamePrefix    - префикс имени функции, обычно задаётся равным имени enum'а, но для каких-то кривых случаев или во избежание коллизий можно задать что-то иное
    stringTypeName  - имя типа строки, такое, чтобы могло бы быть частью имени функции: ConstCharPtr
 */
#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName )     \
        UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU_IMPL( static, stringType, enumType, fnNamePrefix, stringTypeName )

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
#if !defined(UMBA_MCU_USED)

    #define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2( stringType, enumType, fnNamePrefix, stringTypeName, ... )                                \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING( stringType, enumType, fnNamePrefix, stringTypeName, __VA_ARGS__)         \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP(stringType, enumType, fnNamePrefix, stringTypeName)              \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP(stringType, enumType, fnNamePrefix, stringTypeName)

    #define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName, ... )                         \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName, __VA_ARGS__)  \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP_MEMBER(stringType, enumType, fnNamePrefix, stringTypeName)       \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MAP_MEMBER(stringType, enumType, fnNamePrefix, stringTypeName)

#else

    #define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2( stringType, enumType, fnNamePrefix, stringTypeName, ... )                                 \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING( stringType, enumType, fnNamePrefix, stringTypeName, __VA_ARGS__)          \
                 /*UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP(stringType, enumType, fnNamePrefix, stringTypeName)*/           \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU(stringType, enumType, fnNamePrefix, stringTypeName)

    #define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName, ... )                          \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MEMBER( stringType, enumType, fnNamePrefix, stringTypeName, __VA_ARGS__)   \
                 /*UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_TO_STRING_MAKE_MAP_MEMBER(stringType, enumType, fnNamePrefix, stringTypeName)*/    \
                 UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_IMPL_FROM_STRING_IMPL_V_MCU_MEMBER(stringType, enumType, fnNamePrefix, stringTypeName)

#endif


#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX( stringType, enumType, fnNamePrefix, ... ) \
            UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2( stringType, enumType, fnNamePrefix, stringType, __VA_ARGS__ )

#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS( stringType, enumType, ... ) \
            UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2( stringType, enumType, enumType, stringType, __VA_ARGS__ )


#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX_MEMBER( stringType, enumType, fnNamePrefix, ... ) \
            UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2_MEMBER( stringType, enumType, fnNamePrefix, stringType, __VA_ARGS__ )

#define UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_MEMBER( stringType, enumType, ... ) \
            UMBA_ENUM_CLASS_IMPLEMENT_STRING_CONVERTERS_EX2_MEMBER( stringType, enumType, enumType, stringType, __VA_ARGS__ )







