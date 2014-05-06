#pragma once

#ifdef _MSC_VER
// dirty hack for MS Compiler
# ifndef noexcept
#  define _ALLOW_KEYWORD_MACROS
#  define noexcept throw()
# endif
#endif

#include <luasel/details/Function.hpp>
