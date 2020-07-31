#include "default_signature.hlsli"
#include "default_samplers.hlsli"

struct interpolants
{
    float4 position     : SV_POSITION0;
};

cbuffer per_draw_call_material_external : register(b3)
{
    float4 m_color;
};


[RootSignature( MyRS1 ) ]
float4 main( interpolants r ) : SV_Target0
{
    return m_color;
}
