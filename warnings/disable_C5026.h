/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief

    Repository: https://github.com/al-martyn1/umba
*/

#if defined(_MSC_VER)
    #if defined(UMBA_ENABLE_WARNING_DISABLE_VERBOSE)
        //#pragma message("!!! This function or variable may be unsafe")
        // #pragma message("!!! Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified")
    #endif
    #pragma warning(disable:5026) // - warning C5026: 'IDENT': move constructor was implicitly defined as deleted
#endif
