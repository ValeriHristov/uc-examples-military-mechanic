#include "default_signature.hlsli"
#include "soldier_skin.hlsli"

namespace interop
{
    struct skinned_draw_constants
    {
        float4x4 m_joints_palette[127];
    };
}

cbuffer bone_count : register(b0)
{
    uint  g_bone_count;
};

cbuffer per_draw_call_external : register(b1)
{
    interop::skinned_draw_constants  g_skinned_constants;
};

RWByteAddressBuffer b : register(u0);

[numthreads(64, 1 , 1 ) ]
[RootSignature( MyRS2 ) ]
void main(uint3 v : SV_DispatchThreadID)
{
    uint bone   = v.x;
    if (bone < g_bone_count)
    {
        float4x4 m = g_skinned_constants.m_joints_palette[bone];

        float3x3    r;
        float3      t;

        decompose(m, r, t);
        quaternion  q = quat(r);
        dual_quaternion dq0 = dual_quat(q, t);

        b.Store4(bone * 32,      asuint(dq0.m_real.m_v));
        b.Store4(bone * 32 + 16, asuint(dq0.m_dual.m_v));

        //float4 r0 = m[0];
        //float4 r1 = m[3];
 
        //b.Store4(bone * 32,      asuint(r0));
        //b.Store4(bone * 32 + 16, asuint(r1));
    }
}





