/*! \file
    \author Alexander Martynov (Marty AKA al-martyn1) <amart@mail.ru>
    \copyright (c) 2018-2026 Alexander Martynov
    \brief

    Repository: https://github.com/al-martyn1/umba
*/

#include "push.h"

#if defined(_MSC_VER)

    #include "disable_C4100.h"
    #include "disable_C4189.h"
    #include "disable_C4267.h"
    #include "disable_C4365.h"
    #include "disable_C4623.h"
    #include "disable_C4625.h"
    #include "disable_C4668.h"
    #include "disable_C4800.h"
    #include "disable_C4946.h"
    #include "disable_C5026.h"
    #include "disable_C5243.h"

#elif defined(__GNUC__)

    #include "disable_deprecated_declarations.h"
    #include "disable_unused_parameter.h"

#endif

