/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_QUATERNION_H__
#define __DAVAENGINE_QUATERNION_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Math/Matrix4.h"
#include "Math/MathHelpers.h"

namespace DAVA
{
/** 
    \ingroup math
    \brief Class that represents quaternion. Main purpose of the class to give you ability to interpolate between 3D orientations.
    It used inside skeleton animations, to make all animations smooth and frame intedepndant. 
*/
class Quaternion
{
public:
    union
    {
        struct
        {
            float32 x, y, z, w;
        };
        float32 data[4];
    };

    inline Quaternion(float32 _x = 0.0f, float32 _y = 0.0f, float32 _z = 0.0f, float32 _w = 1.0f);
    inline Quaternion(const Quaternion& _q);
    inline const Quaternion& operator=(const Quaternion& _q);

    inline Matrix4 GetMatrix() const;
    inline void GetMatrix(Matrix4* m) const;

    inline float32 Lenght() const;
    inline void Normalize();
    inline void Transpose();
    inline void Inverse();

    inline void Set(float32 _x = 0.0f, float32 _y = 0.0f, float32 _z = 0.0f, float32 _w = 1.0f);

    // Construct
    inline static Quaternion MakeRotation(const Vector3& euler);
    inline static Quaternion MakeRotation(const Vector3& vector, float32 Angle);
    inline static Quaternion MakeRotation(const Matrix4& matrix);
    inline static Quaternion MakeRotation(const Vector3& source, const Vector3& dest, const Vector3& fallbackAxis = Vector3(0, 0, 0));

    inline static Quaternion MakeRotationFast(const Vector3& vector, float32 angle);
    inline static Quaternion MakeRotationFastX(float32 angle);
    inline static Quaternion MakeRotationFastY(float32 angle);
    inline static Quaternion MakeRotationFastZ(float32 angle);

    inline void Construct(const Vector3& euler);
    inline void Construct(const Vector3& vector, float32 Angle);
    inline void Construct(const Matrix4& matrix);
    inline void Construct(const Vector3& source, const Vector3& dest, const Vector3& fallbackAxis = Vector3(0, 0, 0));

    inline void ConstructRotationFast(const Vector3& vector, float32 angle);
    inline void ConstructRotationFastX(float32 angle);
    inline void ConstructRotationFastY(float32 angle);
    inline void ConstructRotationFastZ(float32 angle);

    // Convert To
    inline void ConvertToAxisAngle(Vector3& vector, float32& Angle) const;

    // Fast methods
    inline void Mul(const Quaternion* mul, Quaternion* res) const;

    inline Quaternion& operator*=(const Quaternion& q);
    inline Quaternion operator*(const Quaternion& q) const;

    inline float32 DotProduct(const Quaternion& q2) const;
    inline void Slerp(const Quaternion& q1, const Quaternion& q2, float32 t);

    inline Vector3 ApplyToVectorFast(const Vector3& inVec) const;

    //! Comparison operators
    inline bool operator==(const Quaternion& _v) const;
    inline bool operator!=(const Quaternion& _v) const;
};

inline void Quaternion::Construct(const Vector3& source, const Vector3& dest, const Vector3& fallbackAxis)
{
    // Based on Stan Melax's article in Game Programming Gems
    // Copy, since cannot modify local
    Vector3 v0 = source;
    Vector3 v1 = dest;
    v0.Normalize();
    v1.Normalize();

    float32 d = v0.DotProduct(v1);
    // If dot == 1, vectors are the same
    if (d >= 1.0f)
    {
        Set(); //Quaternion::IDENTITY;
    }
    if (d < (1e-6f - 1.0f))
    {
        if (fallbackAxis != Vector3(0, 0, 0))
        {
            // rotate 180 degrees about the fallback axis
            Construct(fallbackAxis, PI);
        }
        else
        {
            // Generate an axis
            Vector3 axisX(1, 0, 0);
            Vector3 axis = axisX.CrossProduct(source);
            if (axis.IsZero()) // pick another if colinear
            {
                Vector3 axisY(0, 1, 0);
                axis = axisY.CrossProduct(source);
            }
            axis.Normalize();
            Construct(axis, PI);
        }
    }
    else
    {
        float32 s = sqrt((1 + d) * 2);
        float32 invs = 1 / s;

        Vector3 c = v0.CrossProduct(v1);

        x = c.x * invs;
        y = c.y * invs;
        z = c.z * invs;
        w = s * 0.5f;
        Normalize();
    }
}

// implementation of Quaternion
inline Quaternion::Quaternion(float32 _x, float32 _y, float32 _z, float32 _w)
    : x(_x)
    , y(_y)
    , z(_z)
    , w(_w)
{
}

inline Quaternion::Quaternion(const Quaternion& _q)
{
    x = _q.x;
    y = _q.y;
    z = _q.z;
    w = _q.w;
}

inline const Quaternion& Quaternion::operator=(const Quaternion& _q)
{
    x = _q.x;
    y = _q.y;
    z = _q.z;
    w = _q.w;

    return *this;
}

inline void Quaternion::Set(float32 _x, float32 _y, float32 _z, float32 _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

inline float32 Quaternion::Lenght() const
{
    return sqrtf(x * x + y * y + z * z + w * w);
}

inline void Quaternion::Normalize()
{
    float32 lenght = Lenght();
    x /= lenght;
    y /= lenght;
    z /= lenght;
    w /= lenght;
}

inline void Quaternion::Transpose()
{
    x = -x;
    y = -y;
    z = -z;
}

inline void Quaternion::Inverse()
{
    Transpose(); //
    Normalize(); //
}

inline Matrix4 Quaternion::GetMatrix() const
{
    /*
    [ 1-2y2-2z2     2xy-2wz     2xz+2wy     ]
    [ 2xy+2wz       1-2x2-2z2   2yz-2wx     ]
    [ 2xz-2wy       2yz+2wx     1-2x2-2y2   ]
*/
    Matrix4 m;
    float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
    x2 = x + x;
    y2 = y + y;
    z2 = z + z;
    xx = x * x2;
    xy = x * y2;
    xz = x * z2;
    yy = y * y2;
    yz = y * z2;
    zz = z * z2;
    wx = w * x2;
    wy = w * y2;
    wz = w * z2;

    m._data[0][0] = 1.0f - (yy + zz);
    m._data[1][0] = xy - wz;
    m._data[2][0] = xz + wy;
    m._data[0][1] = xy + wz;
    m._data[1][1] = 1.0f - (xx + zz);
    m._data[2][1] = yz - wx;
    m._data[0][2] = xz - wy;
    m._data[1][2] = yz + wx;
    m._data[2][2] = 1.0f - (xx + yy);

    m._data[3][0] = m._data[3][1] = m._data[3][2] = 0;
    m._data[0][3] = m._data[1][3] = m._data[2][3] = 0;
    m._data[3][3] = 1;

    return m;
}

inline void Quaternion::GetMatrix(Matrix4* m) const
{
    if (!m)
        return;

    float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
    x2 = x + x;
    y2 = y + y;
    z2 = z + z;
    xx = x * x2;
    xy = x * y2;
    xz = x * z2;
    yy = y * y2;
    yz = y * z2;
    zz = z * z2;
    wx = w * x2;
    wy = w * y2;
    wz = w * z2;

    m->_data[0][0] = 1.0f - (yy + zz);
    m->_data[1][0] = xy - wz;
    m->_data[2][0] = xz + wy;
    m->_data[0][1] = xy + wz;
    m->_data[1][1] = 1.0f - (xx + zz);
    m->_data[2][1] = yz - wx;
    m->_data[0][2] = xz - wy;
    m->_data[1][2] = yz + wx;
    m->_data[2][2] = 1.0f - (xx + yy);

    m->_data[3][0] = m->_data[3][1] = m->_data[3][2] = 0;
    m->_data[0][3] = m->_data[1][3] = m->_data[2][3] = 0;
    m->_data[3][3] = 1;
}

inline void Quaternion::Mul(const Quaternion* q2, Quaternion* res) const
{
    const Quaternion* q1 = this;
    float32 A, B, C, D, E, F, G, H;

    A = (q1->w + q1->x) * (q2->w + q2->x);
    B = (q1->z - q1->y) * (q2->y - q2->z);
    C = (q1->x - q1->w) * (q2->y + q2->z);
    D = (q1->y + q1->z) * (q2->x - q2->w);
    E = (q1->x + q1->z) * (q2->x + q2->y);
    F = (q1->x - q1->z) * (q2->x - q2->y);
    G = (q1->w + q1->y) * (q2->w - q2->z);
    H = (q1->w - q1->y) * (q2->w + q2->z);

    res->w = B + (-E - F + G + H) * 0.5f;
    res->x = A - (E + F + G + H) * 0.5f;
    res->y = -C + (E - F + G - H) * 0.5f;
    res->z = -D + (E - F - G + H) * 0.5f;
}

inline Quaternion& Quaternion::operator*=(const Quaternion& q)
{
    this->Mul(&q, this);
    return *this;
}

inline Quaternion Quaternion::operator*(const Quaternion& q) const
{
    Quaternion r;
    this->Mul(&q, &r);
    return r;
}

//according to  Molecule Engine it works 35% faster
//see http://molecularmusings.wordpress.com/2013/05/24/a-faster-quaternion-vector-multiplication/ for details
inline Vector3 Quaternion::ApplyToVectorFast(const Vector3& inVec) const
{
    Vector3 qVec(x, y, z);
    Vector3 t = 2.0 * qVec.CrossProduct(inVec);
    return inVec + w * t + qVec.CrossProduct(t);
}

inline float32 Quaternion::DotProduct(const Quaternion& q2) const
{
    return x * q2.x + y * q2.y + z * q2.z + w * q2.w;
}

inline void Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float32 t)
{
    float32 q2t[4];
    float32 omega, cosom, sinom, scale0, scale1;

    // ÍÓÒËÌÛÒ Û„Î‡
    cosom = q1.DotProduct(q2);

    if (cosom < 0.0f)
    {
        cosom = -cosom;
        q2t[0] = -q2.x;
        q2t[1] = -q2.y;
        q2t[2] = -q2.z;
        q2t[3] = -q2.w;
    }
    else
    {
        q2t[0] = q2.x;
        q2t[1] = q2.y;
        q2t[2] = q2.z;
        q2t[3] = q2.w;
    }

    if ((1.0f - cosom) > 0.05f)
    {
        // ÒÚ‡Ì‰‡ÚÌ˚È ÒÎÛ˜‡È (slerp)
        omega = acosf(cosom);
        sinom = sinf(omega);
        scale0 = sinf((1.0f - t) * omega) / sinom;
        scale1 = sinf(t * omega) / sinom;
    }
    else
    {
        // ÂÒÎË Ï‡ÎÂÌ¸ÍËÈ Û„ÓÎ - ÎËÌÂÈÌ‡ˇ ËÌÚÂÔÓÎˇˆËˇ
        scale0 = 1.0f - t;
        scale1 = t;
    }

    x = scale0 * q1.x + scale1 * q2t[0];
    y = scale0 * q1.y + scale1 * q2t[1];
    z = scale0 * q1.z + scale1 * q2t[2];
    w = scale0 * q1.w + scale1 * q2t[3];
}

inline void Quaternion::Construct(const Vector3& axis, float32 angle)
{
    Vector3 axisR = axis;
    axisR.Normalize();

    float32 halfAngle = 0.5f * angle;
    float32 sin = sinf(halfAngle);
    w = cosf(halfAngle);
    x = sin * axisR.x;
    y = sin * axisR.y;
    z = sin * axisR.z;
}

inline void Quaternion::ConstructRotationFast(const Vector3& vector, float32 angle)
{
    DVASSERT(FLOAT_EQUAL(vector.SquareLength(), 1.f));

    float32 sine;
    SinCosFast(angle * .5f, sine, w);
    x = sine * vector.x;
    y = sine * vector.y;
    z = sine * vector.z;
}

inline void Quaternion::ConstructRotationFastX(float32 angle)
{
    y = z = 0.f;
    SinCosFast(angle * .5f, x, w);
}

inline void Quaternion::ConstructRotationFastY(float32 angle)
{
    x = z = 0.f;
    SinCosFast(angle * .5f, y, w);
}

inline void Quaternion::ConstructRotationFastZ(float32 angle)
{
    x = y = 0.f;
    SinCosFast(angle * .5f, z, w);
}

inline void Quaternion::Construct(const Vector3& euler)
{
    Quaternion x_q, y_q, z_q, v;
    x_q.Construct(Vector3::UnitX, euler.x);
    y_q.Construct(Vector3::UnitY, euler.y);
    z_q.Construct(Vector3::UnitZ, euler.z);

    *this = x_q;
    Mul(&y_q, &v);
    v.Mul(&z_q, this);
}

inline void Quaternion::Construct(const Matrix4& matrix)
{
    //float tr = m[0][0] + m[1][1] + m[2][2]; // trace of martix
    //if (tr > 0.0f){     // if trace positive than "w" is biggest component
    //  Set( m[1][2] - m[2][1], m[2][0] - m[0][2], m[0][1] - m[1][0], tr + 1.0f );
    //}else                 // Some of vector components is bigger
    //{
    //  if( (m[0][0] > m[1][1] ) && ( m[0][0] > m[2][2]) ) {
    //      Set( 1.0f + m[0][0] - m[1][1] - m[2][2], m[1][0] + m[0][1],
    //          m[2][0] + m[0][2], m[1][2] - m[2][1] );
    //  }else
    //      if ( m[1][1] > m[2][2] ){
    //          Set( m[1][0] + m[0][1], 1.0f + m[1][1] - m[0][0] - m[2][2],
    //              m[2][1] + m[1][2], m[2][0] - m[0][2] );
    //      }else{
    //          Set( m[2][0] + m[0][2], m[2][1] + m[1][2],
    //              1.0f + m[2][2] - m[0][0] - m[1][1], m[0][1] - m[1][0] );
    //      }
    //}
    using mtx_elm = float32[4][4];
    const mtx_elm& m = matrix._data;

    float tr, s, q[4];
    int i, j, k;

    int nxt[3] = { 1, 2, 0 };

    tr = m[0][0] + m[1][1] + m[2][2];

    if (tr > 0.0f)
    {
        s = sqrtf(tr + 1.0f);
        w = s / 2.0f;
        s = 0.5f / s;
        x = (m[1][2] - m[2][1]) * s;
        y = (m[2][0] - m[0][2]) * s;
        z = (m[0][1] - m[1][0]) * s;
    }
    else
    {
        i = 0;
        if (m[1][1] > m[0][0])
            i = 1;
        if (m[2][2] > m[i][i])
            i = 2;
        j = nxt[i];
        k = nxt[j];

        s = sqrtf((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

        q[i] = s * 0.5f;

        if (s != 0.0f)
            s = 0.5f / s;

        q[3] = (m[j][k] - m[k][j]) * s;
        q[j] = (m[i][j] + m[j][i]) * s;
        q[k] = (m[i][k] + m[k][i]) * s;

        x = q[0];
        y = q[1];
        z = q[2];
        w = q[3];
    }
}

inline void Quaternion::ConvertToAxisAngle(Vector3& axis, float32& angle) const
{
    float vl = sqrtf(x * x + y * y + z * z);
    if (vl > 0.01f)
    {
        float ivl = 1.0f / vl;
        axis = Vector3(x * ivl, y * ivl, z * ivl);
        if (w < 0)
            angle = 2.0f * atan2f(-vl, -w); //-PI,0
        else
            angle = 2.0f * atan2f(vl, w); //0,PI
    }
    else
    {
        axis = Vector3(0, 0, 0);
        angle = 0;
    }
};

//! Comparison operators
inline bool Quaternion::operator==(const Quaternion& _v) const
{
    return (Memcmp(data, _v.data, sizeof(Quaternion)) == 0);
}
inline bool Quaternion::operator!=(const Quaternion& _v) const
{
    return (Memcmp(data, _v.data, sizeof(Quaternion)) != 0);
}

inline Quaternion Quaternion::MakeRotation(const Vector3& euler)
{
    Quaternion ret;

    return ret;
}
inline Quaternion Quaternion::MakeRotation(const Vector3& vector, float32 Angle)
{
    Quaternion ret;
    ret.Construct(vector, Angle);
    return ret;
}
inline Quaternion Quaternion::MakeRotation(const Matrix4& matrix)
{
    Quaternion ret;
    ret.Construct(matrix);
    return ret;
}
inline Quaternion Quaternion::MakeRotation(const Vector3& source, const Vector3& dest, const Vector3& fallbackAxis)
{
    Quaternion ret;
    ret.Construct(source, dest, fallbackAxis);
    return ret;
}

inline Quaternion Quaternion::MakeRotationFast(const Vector3& vector, float32 angle)
{
    Quaternion ret;
    ret.ConstructRotationFast(vector, angle);
    return ret;
}
inline Quaternion Quaternion::MakeRotationFastX(float32 angle)
{
    Quaternion ret;
    ret.ConstructRotationFastX(angle);
    return ret;
}
inline Quaternion Quaternion::MakeRotationFastY(float32 angle)
{
    Quaternion ret;
    ret.ConstructRotationFastY(angle);
    return ret;
}
inline Quaternion Quaternion::MakeRotationFastZ(float32 angle)
{
    Quaternion ret;
    ret.ConstructRotationFastZ(angle);
    return ret;
}
};

#endif // __DAVAENGINE_QUATERNION_H__