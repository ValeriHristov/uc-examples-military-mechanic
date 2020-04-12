#include "default_signature.hlsli"

#include "vector_space.hlsli"
#include "frame.hlsli"

#include "interop.h"

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

static const uint position_stride = 12;

ByteAddressBuffer b : register(t0);

float3 load_vertex(uint v)
{
    return asfloat(b.Load3(m_draw_call.m_position + v * position_stride));
}

[RootSignature( MyRS1 ) ]
interpolants main(uint v : SV_VERTEXID)
{
    interpolants r = (interpolants)0;
    input i;

    i.position              = load_vertex(v);

    point_os position_os    = make_point_os(i.position);

    euclidean_transform_3d  world = m_draw_call.m_world;
    
    r.position              = project_p_os(position_os, world, m_frame.m_view, m_frame.m_perspective).m_value;
    return r;
}





