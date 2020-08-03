#include "default_signature.hlsli"
#include "default_samplers.hlsli"

struct interpolants
{
    float4 position     : SV_POSITION0;
    float2 uv           : texcoord0;
    float3 normal       : NORMAL0;
};

float checker_board_pattern(float2 uv)
{
    float2 c = floor(uv) / 2;
    return frac(c.x + c.y) * 2;
}

float4 checker_board(float2 uv)
{
    float2 uv_scaled = uv * float2(32.0f, 32.0f);
    float checker = checker_board_pattern(uv_scaled);
    return float4(checker, checker, checker, 1.0f);
}

Texture2D<float4> t         : register(t1);

[RootSignature( MyRS1 ) ]
float4 main( interpolants r ) : SV_Target0
{
    float3 l    = normalize(float3( 0, 1,1));
    float3 n    = normalize(r.normal);
    float2 te   = float2(r.uv.x, r.uv.y);

    return t.Sample(g_linear_clamp , te) * dot(n,l);
}
