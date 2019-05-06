#include "default_signature.hlsli"
#include "default_samplers.hlsli"

struct interpolated_value
{
    float4 m_position : SV_POSITION;
    float3 m_color    : TEXCOORD0;
    float2 m_uv       : TEXCOORD1;
};

Texture2D<float> b : register(t2);

[RootSignature( MyRS1 ) ]
float4 main(interpolated_value v) : SV_TARGET0
{
    return float4(v.m_color, 0.0f) * b.Sample(g_point_clamp, v.m_uv ) ;
}
