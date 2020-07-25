#ifndef __math_intrinsics_hlsli__
#define __math_intrinsics_hlsli__

float max3(float a, float b, float c)
{
    return max(a, max(b,c));
}

float max3(float3 a)
{
    return max(a.x, max(a.y,a.z));
}

float2 max3(float2 a, float2 b, float2 c)
{
    return float2( max(a.x, max(b.x,c.x)), max(a.y, max(b.y,c.y)));
}

float3 max3(float3 a, float3 b, float3 c)
{
    return float3( max(a.x, max(b.x,c.x)), max(a.y, max(b.y,c.y)), max(a.z, max(b.z,c.z)));
}

float4 max3(float4 a, float4 b, float4 c)
{
    return float4( max(a.x, max(b.x,c.x)), max(a.y, max(b.y,c.y)), max(a.z, max(b.z,c.z)), max(a.w, max(b.w,c.w)));
}

#endif
