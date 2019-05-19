#ifndef __interop_h__
#define __interop_h__

#if !defined(__cplusplus)
typedef uint uint32_t;
#else
#include <uc/math/math.h>
#endif


namespace interop
{
    #if defined(__cplusplus)
    using namespace uc::math;
    #endif

    typedef uint32_t           offset;

    struct frame
    {
        euclidean_transform_3d   m_view;
        projective_transform_3d  m_perspective;
    };

    struct draw_call
    {
        offset                 m_batch;
        offset                 m_start_vertex;

		offset                 m_pad0;
		offset                 m_pad1;

        offset                 m_position;
        offset                 m_uv;

        offset                 m_normal;
        offset                 m_tangent;

        euclidean_transform_3d m_world;
    };
}

#endif