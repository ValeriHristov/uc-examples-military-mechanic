#include "default_signature.hlsli"

#include "vector_space.hlsli"
#include "frame.hlsli"

#include "interop.h"

struct interpolants
{
    float4 position     : SV_POSITION0;
    float2 uv           : texcoord0;
};

struct input
{
    float3 position : position;
    float2 uv       : texcoord0;
};

cbuffer per_draw_call : register(b0)
{
    interop::draw_call m_draw_call;
};

cbuffer per_draw_call_additional    : register(b2)
{
    interop::skinned_draw_constants  m_draw_constants;
};

static const uint position_stride = 12;
static const uint normal_stride   = 12;
static const uint tangent_stride  = 12;
static const uint uv_stride       = 8;

ByteAddressBuffer b : register(t0);

float3 load_vertex(uint v)
{
    return asfloat(b.Load3(m_draw_call.m_position + v * position_stride));
}

float2 load_uv(uint v)
{
    return asfloat(b.Load2(m_draw_call.m_uv + v * uv_stride));
}

[RootSignature( MyRS1 ) ]
interpolants main(uint v : SV_VERTEXID)
{
    interpolants r = (interpolants)0;

    uint positionStride = 16;
    uint colorStride    = 12;
    uint positionOffset = 0;
    uint colorOffset    = 16 * 3;

    input i;

    i.position              = load_vertex(v);
    i.uv                    = load_uv(v);

    point_os position_os    = make_point_os(i.position);

    euclidean_transform_3d  world = m_draw_call.m_world;
    
    r.uv                    = i.uv;
    r.position              = project_p_os(position_os, world, m_frame.m_view, m_frame.m_perspective).m_value;
    return r;
    
}





