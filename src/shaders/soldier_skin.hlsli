#ifndef __transform_skinned_hlsli__
#define __transform_skinned_hlsli__

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

float3 skin_position_scale(float3 position, float3x3 scale, float weight)
{
    return mul(position, scale) * weight;
}

float3 skin_position_rotation(float3 position, float3x3 rotation, float weight)
{
    return mul(position, rotation) * weight;
}

float3 skin_position_translation(float3 position, float4 translation, float weight)
{
    return (position + translation.xyz) * weight;
}

float4 skin_position2(float4 position, float4 weights, float4 indices, float3x3 scales[127], float3x3 rotations[127], float4 translations[127])
{
    float3 skinned_position_scale;

    skinned_position_scale       = skin_position_scale(position.xyz, scales[indices.x], weights.x);
    skinned_position_scale      += skin_position_scale(position.xyz, scales[indices.y], weights.y);
    skinned_position_scale      += skin_position_scale(position.xyz, scales[indices.z], weights.z);
    skinned_position_scale      += skin_position_scale(position.xyz, scales[indices.w], weights.w);
    skinned_position_scale      /= (weights.x + weights.y + weights.z + weights.w);

    float3 skinned_position_rotation;

    skinned_position_rotation    = skin_position_rotation(skinned_position_scale, rotations[indices.x], weights.x);
    skinned_position_rotation   += skin_position_rotation(skinned_position_scale, rotations[indices.y], weights.y);
    skinned_position_rotation   += skin_position_rotation(skinned_position_scale, rotations[indices.z], weights.z);
    skinned_position_rotation   += skin_position_rotation(skinned_position_scale, rotations[indices.w], weights.w);
    skinned_position_rotation   /= (weights.x + weights.y + weights.z + weights.w);


    float3 skinned_position_translation;

    skinned_position_translation  = skin_position_translation(skinned_position_rotation, translations[indices.x], weights.x);
    skinned_position_translation += skin_position_translation(skinned_position_rotation, translations[indices.y], weights.y);
    skinned_position_translation += skin_position_translation(skinned_position_rotation, translations[indices.z], weights.z);
    skinned_position_translation += skin_position_translation(skinned_position_rotation, translations[indices.w], weights.w);
    skinned_position_translation /= (weights.x + weights.y + weights.z + weights.w);

    return float4(skinned_position_translation.xyz, 1.0);
}



#endif
