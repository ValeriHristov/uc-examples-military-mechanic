#ifndef __interop_float_h__
#define __interop_float_h__

#if defined(__cplusplus)
#include <uc/math/math.h>
#include <type_traits>
#endif

namespace interop
{
    #if defined(__cplusplus)
    using namespace uc::math;
    #endif
    struct float4x2
    {
        float4 m_b0;
        float4 m_b1;
    };
}

#endif