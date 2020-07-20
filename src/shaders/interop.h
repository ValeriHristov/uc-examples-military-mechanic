#ifndef __interop_h__
#define __interop_h__

#if !defined(__cplusplus)
typedef uint uint32_t;
#else
#include <uc/math/math.h>
#include <type_traits>
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

        offset                 m_blend_weights;
        offset                 m_blend_indices;

        offset                 m_position;
        offset                 m_uv;

        offset                 m_normal;
        offset                 m_tangent;


        euclidean_transform_3d m_world;
    };

    #if defined(__cplusplus)
    static_assert(sizeof(draw_call) <= 32 * 4, "64-bit code generation is not supported.");
    #endif

    #if defined(__cplusplus)
    struct alignas(4) float3x3
    {
        float   r[9];
    };
    #endif

    struct skinned_draw_constants
    {
        float4x4 m_joints_palette[127];
        float4   m_translations[127];
        float3x3 m_rotations[127];
        float3x3 m_scales[127];
    };
}

#endif