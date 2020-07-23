#ifndef __dual_quaternion_hlsli__
#define __dual_quaternion_hlsli__

struct quaternion
{
    float4 m_v;

    float  x() { return m_v.x;}
    float  y() { return m_v.y;}
    float  z() { return m_v.z;}
    float  w() { return m_v.w;}

    float  i() { return m_v.x;}
    float  j() { return m_v.y;}
    float  k() { return m_v.z;}

    float3 xyz() { return float3(m_v.xyz); }
};

quaternion conjugate( quaternion q)
{
    quaternion r;
    r.m_v = float4(-1,-1,-1,1) * q.m_v;
    return r;
}

quaternion mul(quaternion a, quaternion b)
{
    float x = a.w() * b.x() + a.x() * b.w()  + a.y() * b.z() - a.z() * b.y();
    float y = a.w() * b.y() - a.x() * b.z()  + a.y() * b.w() + a.z() * b.x();
    float z = a.w() * b.z() + a.x() * b.y()  - a.y() * b.x() + a.z() * b.w();
    float w = a.w() * b.w() - a.x() * b.x()  - a.y() * b.y() - a.z() * b.z();

    quaternion r ;
    r.m_v = float4(x,y,z,w);
    return r;
}

float dot(quaternion q1, quaternion q2)
{
    return dot(q1.m_v, q2.m_v);
}

float norm(quaternion q)
{
    return sqrt(dot(q.m_v, q.m_v));
}

quaternion normalize( quaternion q)
{
    float n     = norm(q);
    quaternion  r;
    r.m_v       = q.m_v / n;
    return r;
}

quaternion add( quaternion q1, quaternion q2)
{
    quaternion r;

    r.m_v.x = q1.m_v.x + q2.m_v.x;
    r.m_v.y = q1.m_v.y + q2.m_v.y;
    r.m_v.z = q1.m_v.z + q2.m_v.z;
    r.m_v.w = q1.m_v.w + q2.m_v.w;

    return r;
}

quaternion mul( quaternion q1, float scalar)
{
    quaternion r;
    r.m_v   = q1.m_v * scalar;
    return r;
}

float3 rotate(quaternion q, float3 v)
{
    float3 t = 2 * cross(q.xyz(), v);
    return v + q.m_v.w * t + cross(q.xyz(), t);
}

float4x4 mat4x4(quaternion q)
{
    float xy = 2 * q.x() * q.y();
    float xz = 2 * q.x() * q.z();
    float yz = 2 * q.y() * q.z();

    float wx = 2 * q.w() * q.x();
    float wy = 2 * q.w() * q.y();
    float wz = 2 * q.w() * q.z();

    float x2 = 2 * q.x() * q.x();
    float y2 = 2 * q.y() * q.y();
    float z2 = 2 * q.z() * q.z();

    float4x4 r;

    r[0]    = float4(1 - y2 - z2, xy + wz,     wz-wy,  0);
    r[1]    = float4(xy-wz,       1- x2-z2,    yz-wx,  0);
    r[2]    = float4(xz+wy,       yz-wx,       1-x2-y2,0);
    r[3]    = float4(0,           0,           0,      1);

    return  r;
}

float3x3 mat3x3(quaternion q)
{
    float xy = 2 * q.x() * q.y();
    float xz = 2 * q.x() * q.z();
    float yz = 2 * q.y() * q.z();

    float wx = 2 * q.w() * q.x();
    float wy = 2 * q.w() * q.y();
    float wz = 2 * q.w() * q.z();

    float x2 = 2 * q.x() * q.x();
    float y2 = 2 * q.y() * q.y();
    float z2 = 2 * q.z() * q.z();

    float3x3 r;

    r[0]    = float3(1 - y2 - z2, xy + wz,     wz-wy);
    r[1]    = float3(xy-wz,       1- x2-z2,    yz-wx);
    r[2]    = float3(xz+wy,       yz-wx,       1-x2-y2);

    return  r;
}

quaternion quat(float4x4 m)
{           
    float X = m[0].x;
    float Y = m[1].y;
    float Z = m[2].z;
    float W = 1.0f;

    float T = X + Y + Z;

    //m00, m01, m02, m03
    //m10, m11, m12, m13
    //m20, m21, m22, m23
    //m30, m31, m32, m33

    float4 rv;

    if (T > 0.0f)
    {

        float k = sqrt(T + W);
        float w = k / 2.0f;

        float m_21 = m[2].y;
        float m_12 = m[1].z;

        float m_02 = m[0].z;
        float m_20 = m[2].x;

        float m_10 = m[1].x;
        float m_01 = m[0].y;

        float x = (-m_21 + m_12);
        float y = (-m_02 + m_20);
        float z = (-m_10 + m_01);

        x /= 2 * k;
        y /= 2 * k;
        z /= 2 * k;

        rv = float4(x, y, z, w);
    }
    else
    {
        float m_21 = m[2].y;
        float m_12 = m[1].z;

        float m_02 = m[0].z;
        float m_20 = m[2].x;

        float m_10 = m[1].x;
        float m_01 = m[0].y;

        float x = (m_21 + m_12);
        float y = (m_02 + m_20);
        float z = (m_10 + m_01);

        float w_0 = (-m_21 + m_12);
        float w_1 = (-m_02 + m_20);
        float w_2 = (-m_10 + m_01);

        if (X > Y)
        {
            if (X > Z)
            {
                //0, 1, 2
                //q[0]

                float v   = sqrt(X - Y - Z + W) / 2.0f;
                float q_0 = v;
                float q_1 = 0.25f  * z / q_0;
                float q_2 = 0.25f  * y / q_0;
                float q_3 = 0.25f  * w_0 / q_0;
                rv = float4(q_0, q_1, q_2, q_3);
            }
            else
            {
                //q[2]

                float v = sqrt(-X - Y + Z + W) / 2.0f;

                float q_2 = v;
                float q_0 = 0.25f  * y / q_2;
                float q_1 = 0.25f  * x / q_2;
                float q_3 = 0.25f  * w_2 / q_2;

                rv = float4(q_0, q_1, q_2, q_3);
            }

        }
        else
        {
            if (Y > Z)
            {
                //q[1]
                float v = sqrt(-X + Y - Z + W) / 2.0f;

                float q_1 = v;

                float q_0 = 0.25f  * z   / q_1;
                float q_2 = 0.25f  * x   / q_1;
                float q_3 = 0.25f  * w_1 / q_1;

                rv = float4(q_0, q_1, q_2, q_3);
            }
            else
            {
                //q[2]

                float v     = sqrt(-X - Y + Z + W) / 2.0f;
                float q_2   = v;

                float q_0   = 0.25f  * y / q_2;
                float q_1   = 0.25f  * x / q_2;
                float q_3   = 0.25f  * w_2 / q_2;
                rv = float4(q_0, q_1, q_2, q_3);
            }
        }
    }

    quaternion r;
    r.m_v = rv;
    return  r;
}

quaternion quat1(float3x3 m)
{           
    //todo: revise this
    //https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
    float X = m[0].x;
    float Y = m[1].y;
    float Z = m[2].z;
    float W = 1.0f;

    float T = X + Y + Z;

    //m00, m01, m02, m03
    //m10, m11, m12, m13
    //m20, m21, m22, m23
    //m30, m31, m32, m33

    float4 rv;

    if (T > 0.0f)
    {

        float k = sqrt(T + W);
        float w = k / 2.0f;

        float m_21 = m[2].y;
        float m_12 = m[1].z;

        float m_02 = m[0].z;
        float m_20 = m[2].x;

        float m_10 = m[1].x;
        float m_01 = m[0].y;

        float x = (-m_21 + m_12);
        float y = (-m_02 + m_20);
        float z = (-m_10 + m_01);

        x /= 2 * k;
        y /= 2 * k;
        z /= 2 * k;

        rv = float4(x, y, z, w);
    }
    else
    {
        float m_21 = m[2].y;
        float m_12 = m[1].z;

        float m_02 = m[0].z;
        float m_20 = m[2].x;

        float m_10 = m[1].x;
        float m_01 = m[0].y;

        float x = (m_21 + m_12);
        float y = (m_02 + m_20);
        float z = (m_10 + m_01);

        float w_0 = (-m_21 + m_12);
        float w_1 = (-m_02 + m_20);
        float w_2 = (-m_10 + m_01);

        if (X > Y)
        {
            if (X > Z)
            {
                //0, 1, 2
                //q[0]

                float v   = sqrt(X - Y - Z + W) / 2.0f;
                float q_0 = v;
                float q_1 = 0.25f  * z / q_0;
                float q_2 = 0.25f  * y / q_0;
                float q_3 = 0.25f  * w_0 / q_0;
                rv = float4(q_0, q_1, q_2, q_3);
            }
            else
            {
                //q[2]

                float v = sqrt(-X - Y + Z + W) / 2.0f;

                float q_2 = v;
                float q_0 = 0.25f  * y / q_2;
                float q_1 = 0.25f  * x / q_2;
                float q_3 = 0.25f  * w_2 / q_2;

                rv = float4(q_0, q_1, q_2, q_3);
            }

        }
        else
        {
            if (Y > Z)
            {
                //q[1]
                float v = sqrt(-X + Y - Z + W) / 2.0f;

                float q_1 = v;

                float q_0 = 0.25f  * z   / q_1;
                float q_2 = 0.25f  * x   / q_1;
                float q_3 = 0.25f  * w_1 / q_1;

                rv = float4(q_0, q_1, q_2, q_3);
            }
            else
            {
                //q[2]

                float v     = sqrt(-X - Y + Z + W) / 2.0f;
                float q_2   = v;

                float q_0   = 0.25f  * y / q_2;
                float q_1   = 0.25f  * x / q_2;
                float q_3   = 0.25f  * w_2 / q_2;
                rv = float4(q_0, q_1, q_2, q_3);
            }
        }
    }

    quaternion r;
    r.m_v = rv;
    return  r;
}

quaternion quat(float3x3 m)
{           
    const float m00 = m._11;
    const float m01 = m._12;
    const float m02 = m._13;

    const float m10 = m._21;
    const float m11 = m._22;
    const float m12 = m._23;

    const float m20 = m._31;
    const float m21 = m._32;
    const float m22 = m._33;

    float t = 0;
    quaternion q;
    if ( m22 < 0.0)
    {
        if ( m00 > m11 )
        {
            t       = 1 + m00 - m11 - m22;
            q.m_v.x = t;
            q.m_v.y = m01 + m10;
            q.m_v.z = m20 + m02;
            q.m_v.w = m12 - m21;
        }
        else
        {
            t       = 1 - m00 + m11 - m22;
            q.m_v.x = m01 + m10;
            q.m_v.y = t;
            q.m_v.z = m12 + m21;
            q.m_v.w = m20 - m02;
            
        }
    }
    else
    {
        if ( m00 < -m11)
        {
            t = 1 - m00 - m11 + m22;
            q.m_v.x = m20 + m02;
            q.m_v.y = m12 + m21;
            q.m_v.z = t;
            q.m_v.w = m01 - m10;
        }
        else
        {
            t = 1 + m00 + m11 + m22;
            q.m_v.x = m12 + m21;
            q.m_v.y = m20 + m02;
            q.m_v.z = m01 - m10;
            q.m_v.w = t;
        }
    }

    quaternion r;
    r.m_v = q.m_v / sqrt(t);
    return  r;
}

struct dual_quaternion
{
    // Non-dual part of the dual quaternion. It also represent the rotation.
    /// @warning If you want to compute the rotation with this don't forget
    /// to normalize the quaternion as it might be the result of a
    /// dual quaternion linear blending
    /// (when overloaded operators like '+' or '*' are used)
    quaternion m_real;

    /// Dual part of the dual quaternion which represent the translation.
    /// translation can be extracted by computing
    /// 2.f * _quat_e * conjugate(_quat_0)
    /// @warning don't forget to normalize quat_0 and quat_e :
    /// quat_0 = quat_0 / || quat_0 || and quat_e = quat_e / || quat_0 ||
    quaternion m_dual;
};

dual_quaternion dual_quat( quaternion rotation, float3 translation)
{
    quaternion q    = normalize(rotation);
    float3     t    = translation;

    float w = -0.5f*( t.x * q.i() + t.y * q.j() + t.z * q.k());
    float i =  0.5f*( t.x * q.w() + t.y * q.k() - t.z * q.j());
    float j =  0.5f*(-t.x * q.k() + t.y * q.w() + t.z * q.i());
    float k =  0.5f*( t.x * q.j() - t.y * q.i() + t.z * q.w());

    dual_quaternion r;
    r.m_real       = rotation;
    r.m_dual.m_v.x = i;
    r.m_dual.m_v.y = j;
    r.m_dual.m_v.z = k;
    r.m_dual.m_v.w = w;
    return r;
}

dual_quaternion mul(dual_quaternion q, float scalar )
{
    dual_quaternion r;

    r.m_real = mul( q.m_real, scalar);
    r.m_dual = mul( q.m_dual, scalar);

    return r;
}

dual_quaternion add(dual_quaternion q1, dual_quaternion q2 )
{
    dual_quaternion r;

    r.m_real = add( q1.m_real, q2.m_real);
    r.m_dual = add( q1.m_real, q2.m_real);

    return r;
}

quaternion rotation(dual_quaternion q)
{
    return normalize(q.m_real);
}

float3 translation(dual_quaternion q)
{
    /// 2.f * _quat_e * conjugate(_quat_0)
    quaternion r = rotation(q);
    quaternion d = normalize(q.m_dual);
    quaternion m = mul(d, conjugate(r));

    return 2 * m.m_v.xyz;
}

float3 transformPoint(dual_quaternion d, float3 p)
{
    // As the dual quaternions may be the results from a
    // linear blending we have to normalize it :
    float n             = rcp( norm(d.m_real) );
    quaternion qblend_0 = mul(d.m_real, n);
    quaternion qblend_e = mul(d.m_dual, n);

    // Translation from the normalized dual quaternion equals :
    // 2.f * qblend_e * conjugate(qblend_0)
    float3 v0           = qblend_0.xyz();
    float3 ve           = qblend_e.xyz();
    float3 trans        = (ve * qblend_0.w() - v0 * qblend_e.w() + cross(v0, ve) ) * 2.f;

    // Rotate
    return rotate(qblend_0, p) + trans;
}

float3 transformVector(dual_quaternion d, float3 v)
{
    return rotate(d.m_real, v);
}

[numthreads(1,1,1)]
void main()
{
    

}


#endif
