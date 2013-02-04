/*!
   @file   CS123Matrix.cpp
   @author Travis Fischer (fisch0920@gmail.com)
   @date   Fall 2008
   
   @brief
      Provides basic functionality for a templated, arbitrarily-sized matrix.
      You will need to fill this file in for the Camtrans assignment.

**/

#include "CS123Algebra.h"
#include <iostream>

//@name Routines which generate specific-purpose transformation matrices
//@{---------------------------------------------------------------------
// @returns the scale matrix described by the vector
Matrix4x4 getScaleMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    return Matrix4x4::diag(v.x,v.y,v.z,1);

}

// @returns the translation matrix described by the vector
Matrix4x4 getTransMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    return Matrix4x4(1,0,0,v.x,
                     0,1,0,v.y,
                     0,0,1,v.z,
                     0,0,0,1);

}

// @returns the rotation matrix about the x axis by the specified angle
Matrix4x4 getRotXMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL cos = std::cos(radians), sin = std::sin(radians);
    return Matrix4x4(1,0,0,0,
                     0,cos,-sin,0,
                     0,sin,cos,0,
                     0,0,0,1);

}

// @returns the rotation matrix about the y axis by the specified angle
Matrix4x4 getRotYMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL cos = std::cos(radians), sin = std::sin(radians);
    return Matrix4x4(cos,0,sin,0,
                     0,1,0,0,
                     -sin,0,cos,0,
                     0,0,0,1);

}

// @returns the rotation matrix about the z axis by the specified angle
Matrix4x4 getRotZMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL cos = std::cos(radians), sin = std::sin(radians);
    return Matrix4x4(cos,-sin,0,0,
                     sin,cos,0,0,
                     0,0,1,0,
                     0,0,0,1);

}

// @returns the rotation matrix around the vector and point by the specified angle
Matrix4x4 getRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL y = atan2(v.z,v.x), z = -atan2(v.y,std::sqrt(SQ(v.x)+SQ(v.z)));
    const Matrix4x4 M1 = getRotYMat(y), M1_inv = getInvRotYMat(y),
                    M2 = getRotZMat(z), M2_inv = getInvRotZMat(z),
                    M3 = getRotXMat(a);
    return getInvTransMat(-p)*M1_inv*M2_inv*M3*M2*M1*getTransMat(-p);

}


// @returns the inverse scale matrix described by the vector
Matrix4x4 getInvScaleMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    return Matrix4x4::diag(1/v.x,1/v.y,1/v.z,1);

}

// @returns the inverse translation matrix described by the vector
Matrix4x4 getInvTransMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    return Matrix4x4(1,0,0,-v.x,
                     0,1,0,-v.y,
                     0,0,1,-v.z,
                     0,0,0,1);

}

// @returns the inverse rotation matrix about the x axis by the specified angle
Matrix4x4 getInvRotXMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL cos = std::cos(radians), sin = std::sin(radians);
    return Matrix4x4(1,0,0,0,
                     0,cos,sin,0,
                     0,-sin,cos,0,
                     0,0,0,1);

}

// @returns the inverse rotation matrix about the y axis by the specified angle
Matrix4x4 getInvRotYMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL cos = std::cos(radians), sin = std::sin(radians);
    return Matrix4x4(cos,0,-sin,0,
                     0,1,0,0,
                     sin,0,cos,0,
                     0,0,0,1);

}

// @returns the inverse rotation matrix about the z axis by the specified angle
Matrix4x4 getInvRotZMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL cos = std::cos(radians), sin = std::sin(radians);
    return Matrix4x4(cos,sin,0,0,
                     -sin,cos,0,0,
                     0,0,1,0,
                     0,0,0,1);


}

// @returns the inverse rotation matrix around the vector and point by the specified angle
Matrix4x4 getInvRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    // @TODO: [CAMTRANS] Fill this in...
    const REAL y = atan2(v.z,v.x), z = -atan2(v.y,std::sqrt(SQ(v.x)+SQ(v.z)));
    const Matrix4x4 M1 = getRotYMat(y), M1_inv = getInvRotYMat(y),
                    M2 = getRotZMat(z), M2_inv = getInvRotZMat(z),
                    M3_inv = getInvRotXMat(a);
    return getInvTransMat(-p)*M1_inv*M2_inv*M3_inv*M2*M1*getTransMat(-p);

}


//@}---------------------------------------------------------------------

