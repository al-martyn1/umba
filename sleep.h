/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief sleepMs

    Repository: https://github.com/al-martyn1/umba
*/

#pragma once

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(WIN32)

    //#include <winsock2.h>

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>

#else

    // Через сокетный select

#endif


//----------------------------------------------------------------------------
namespace umba{


#if defined(_WIN32) || defined(WIN32)

inline
void sleepMs(unsigned sleepTime)
{
    ::Sleep(sleepTime);
}

#else

    // Через сокетный select

#endif


} // namespace umba


