/*!
   @file   CamtransCamera.cpp
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010

   @brief  This is the perspective camera class you will need to fill in for the
   Camtrans assignment.  See the assignment handout for more details.

   For extra credit, you can also create an Orthographic camera. Create another
   class if you want to do that.
*/

#include "CamtransCamera.h"
#include <qgl.h>

CamtransCamera::CamtransCamera() :
        m_eye(0,0,0,1),
        m_look(0,0,-1,0),
        m_up(0,1,0,0),
        m_near(0.0),
        m_far(10.0),
        m_height(M_PI*0.25),
        m_aspect(1.0),
        m_projection(getProjectionMatrix()),
        m_modelview(getModelviewMatrix())
{
    // @TODO: [CAMTRANS] Fill this in...
}

void CamtransCamera::setAspectRatio(float a)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_aspect = a;
    updateProjectionMatrix();
}

Matrix4x4 CamtransCamera::getProjectionMatrix() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_projection;
}

Matrix4x4 CamtransCamera::getCameraMatrix() const
{
    return m_camera;
}

void CamtransCamera::updateProjectionMatrix(void)
{
    // @TODO: [CAMTRANS] Fill this in...
    const double Lh = tan(m_height*M_PI/360.0), Lw = m_aspect*Lh, c = -m_near/m_far;
    const Matrix4x4 Mpt(1,0,0,0,
                        0,1,0,0,
                        0,0,-1/(c+1),c/(c+1),
                        0,0,-1,0);
    m_camera = Matrix4x4::diag(1/Lw, 1/Lh, 1, m_far);
    m_projection = Mpt*m_camera;
}

Matrix4x4 CamtransCamera::getModelviewMatrix() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_modelview;
}

void CamtransCamera::updateModelViewMatrix(void)
{
    // @TODO: [CAMTRANS] Fill this in...
    const Vector4d w = -m_look, v = m_up, u = v.cross(w);
    const Matrix4x4 T = getTransMat(-m_eye),
                    R(u.x, u.y, u.z, 0,
                      v.x, v.y, v.z, 0,
                      w.x, w.y, w.z, 0,
                        0,   0,   0, 1);
    m_modelview = R*T;
}

Vector4 CamtransCamera::getPosition() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_eye;
}

Vector4 CamtransCamera::getLook() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_look;
}

Vector4 CamtransCamera::getUp() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_up;
}

float CamtransCamera::getAspectRatio() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_aspect;
}

float CamtransCamera::getHeightAngle() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_height;
}

void CamtransCamera::orientLook(const Vector4 &eye, const Vector4 &look, const Vector4 &up)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_eye = eye;
    m_look = look.getNormalized();
    m_up = look.cross(up).cross(look).getNormalized();
    updateModelViewMatrix();
}

void CamtransCamera::setHeightAngle(float h)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_height = h;
    updateProjectionMatrix();
}

void CamtransCamera::translate(const Vector4 &v)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_eye += v;
    updateModelViewMatrix();
}

void CamtransCamera::rotateU(float degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 R = getRotMat(Vector4d(), m_up.cross(m_look), degrees*M_PI/180);
    m_up = R*m_up;
    m_look = R*m_look;
    updateModelViewMatrix();
}

void CamtransCamera::rotateV(float degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_look = getRotMat(Vector4d(), m_up, degrees*M_PI/180)*m_look;
    updateModelViewMatrix();
}

void CamtransCamera::rotateN(float degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_up = getRotMat(Vector4d(), -m_look, degrees*M_PI/180)*m_up;
    updateModelViewMatrix();
}

void CamtransCamera::setClip(float nearPlane, float farPlane)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_near = nearPlane;
    m_far = farPlane;
    updateProjectionMatrix();
}