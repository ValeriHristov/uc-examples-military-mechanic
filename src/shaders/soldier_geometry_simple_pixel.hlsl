#include "default_signature.hlsli"
#include "default_samplers.hlsli"
#include "interop.h"

struct interpolants
{
    float4 position     : SV_POSITION0;
};

cbuffer per_draw_call_material_external : register(b3)
{
    interop::skinned_draw_pixel_constants g_material;
};


[RootSignature( MyRS1 ) ]
float4 main( interpolants r ) : SV_Target0
{
    return g_material.m_color;
}
