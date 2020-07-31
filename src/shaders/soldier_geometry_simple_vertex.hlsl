#include "default_signature.hlsli"

#include "vector_space.hlsli"
#include "frame.hlsli"

#include "interop.h"
#include "soldier_skin.hlsli"

struct interpolants
{
    float4 position     : SV_POSITION0;
};

struct input
{
    float3 position : position;
};

cbuffer per_draw_call : register(b0)
{
    interop::draw_call m_draw_call;
};

cbuffer per_draw_call_external : register(b2)
{
    interop::skinned_draw_constants  m_joints_palette;
};

static const uint position_stride       = 12;
static const uint normal_stride         = 12;
static const uint tangent_stride        = 12;
static const uint uv_stride             = 8;
static const uint blend_weights_stride  = 16;
static const uint blend_indices_stride  = 4;

ByteAddressBuffer b : register(t0);

float3 load_vertex(uint v)
{
    return asfloat(b.Load3(m_draw_call.m_position + v * position_stride));
}

float2 load_uv(uint v)
{
    return asfloat(b.Load2(m_draw_call.m_uv + v * uv_stride));
}

float4 load_blend_weights(uint v)
{
    return asfloat(b.Load4(m_draw_call.m_blend_weights + v * blend_weights_stride));
}

uint4 load_blend_indices(uint v)
{
    uint bytes  = b.Load(m_draw_call.m_blend_indices + v * blend_indices_stride);
    uint b0     = (bytes >> 24) & 0xFF;
    uint b1     = (bytes >> 16) & 0xFF;
    uint b2     = (bytes >> 8) & 0xFF;
    uint b3     = (bytes) & 0xFF;

    return uint4( b3, b2, b1, b0);
}

[RootSignature( MyRS1 ) ]
interpolants main(uint v : SV_VERTEXID)
{
    interpolants r = (interpolants)0;

    input i;

    i.position                      = load_vertex(v);
    float4 weights                  = load_blend_weights(v);
    uint4  indices                  = load_blend_indices(v);

    float3 position2                = skin_position(float4(i.position,1.0f), weights, indices, m_joints_palette).xyz;

    point_os position_os            = make_point_os(position2);
    euclidean_transform_3d  world   = m_draw_call.m_world;

    r.position                      = project_p_os(position_os, world, m_frame.m_view, m_frame.m_perspective).m_value;
    
    return r;
}





