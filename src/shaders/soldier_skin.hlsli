#ifndef __transform_skinned_hlsli__
#define __transform_skinned_hlsli__

#define SKIN_LBS1

//todo: replace with quaternions
float4 skin_position(float4 position, float4x4 joint_transform, float weight)
{
    return mul( position, joint_transform) * weight;
}

//todo: rework
float3 skin_normal(float3 normal, float4x4 joint_transform, float weight)
{
    return ( mul( float4(normal, 0.0), joint_transform ) * weight ).xyz;
}

float4 skin_position1(float4 position, float4 weights, float4 indices, float4x4 joints[127])
{
    float4 skinned_position = position;

    skinned_position =  skin_position(position, joints[indices.x], weights.x);
    skinned_position += skin_position(position, joints[indices.y], weights.y);
    skinned_position += skin_position(position, joints[indices.z], weights.z);
    skinned_position += skin_position(position, joints[indices.w], weights.w);

    skinned_position /= (weights.x + weights.y + weights.z + weights.w);

    return skinned_position;
}

float3 skin_normal(float3 normal, float4 weights, float4 indices, float4x4 joints[127])
{
    float3 skinned_normal = normal;

    skinned_normal =  skin_normal(normal, joints[indices.x], weights.x);
    skinned_normal += skin_normal(normal, joints[indices.y], weights.y);
    skinned_normal += skin_normal(normal, joints[indices.z], weights.z);
    skinned_normal += skin_normal(normal, joints[indices.w], weights.w);

    skinned_normal /= (weights.x + weights.y + weights.z + weights.w);

    return skinned_normal;
}

float3 skin_position_scale(float3 position, float4x4 jont_transform, float weight)
{
    float3x3 m;

    m._11 = jont_transform._11;
    m._12 = jont_transform._12;
    m._13 = jont_transform._13;

    m._21 = jont_transform._21;
    m._22 = jont_transform._22;
    m._23 = jont_transform._23;

    m._31 = jont_transform._31;
    m._32 = jont_transform._32;
    m._33 = jont_transform._33;

    return mul( position, m ) * weight;
}


float3 skin_position_translation(float3 position, float4x4 joint_transform, float weight)
{
    float3 translation;

    translation.x = joint_transform._41;
    translation.y = joint_transform._42;
    translation.z = joint_transform._43;

    return (position + translation) * weight;
}

float4 skin_position2(float4 position, float4 weights, float4 indices, float4x4 joints[127])
{
    /*
    float3 skinned_position_0;
    float3 skinned_position_1;
    float3 skinned_position_2;

    skinned_position_0            = skin_position_scale(position.xyz, scales[indices.x], weights.x);
    skinned_position_0           += skin_position_scale(position.xyz, scales[indices.y], weights.y);
    skinned_position_0           += skin_position_scale(position.xyz, scales[indices.z], weights.z);
    skinned_position_0           += skin_position_scale(position.xyz, scales[indices.w], weights.w);
//    skinned_position_0           /= (weights.x + weights.y + weights.z + weights.w);

    skinned_position_1           = skin_position_rotation(skinned_position_0, rotations[indices.x], weights.x);
    skinned_position_1          += skin_position_rotation(skinned_position_0, rotations[indices.y], weights.y);
    skinned_position_1          += skin_position_rotation(skinned_position_0, rotations[indices.z], weights.z);
    skinned_position_1		+= skin_position_rotation(skinned_position_0, rotations[indices.w], weights.w);
 //   skinned_position_1          /= (weights.x + weights.y + weights.z + weights.w);

    skinned_position_2            = skin_position_translation(skinned_position_1, translations[indices.x], weights.x);
    skinned_position_2           += skin_position_translation(skinned_position_1, translations[indices.y], weights.y);
    skinned_position_2		 += skin_position_translation(skinned_position_1, translations[indices.z], weights.z);
    skinned_position_2           += skin_position_translation(skinned_position_1, translations[indices.w], weights.w);
    skinned_position_2 		 /= (weights.x + weights.y + weights.z + weights.w);
    */

    float3 skinned_position_0;
    float3 skinned_position_1;
    float3 skinned_position_2;

    skinned_position_0            = skin_position_scale(position.xyz, joints[indices.x], weights.x);
    skinned_position_0           += skin_position_scale(position.xyz, joints[indices.y], weights.y);
    skinned_position_0           += skin_position_scale(position.xyz, joints[indices.z], weights.z);
    skinned_position_0           += skin_position_scale(position.xyz, joints[indices.w], weights.w);

    skinned_position_1            = skin_position_translation(skinned_position_0, joints[indices.x], weights.x);
    skinned_position_1           += skin_position_translation(skinned_position_0, joints[indices.y], weights.y);
    skinned_position_1           += skin_position_translation(skinned_position_0, joints[indices.z], weights.z);
    skinned_position_1           += skin_position_translation(skinned_position_0, joints[indices.w], weights.w);

    skinned_position_1 		 /= (weights.x + weights.y + weights.z + weights.w);

    return float4(skinned_position_1.xyz, 1.0);
}



#endif
