
#include "lib/CS123SceneData.h"

Matrix4x4 CS123SceneTransformation::MatrixTransform(void) const
{
    switch(type)
    {
    case TRANSFORMATION_TRANSLATE:      return getTransMat(translate);
    case TRANSFORMATION_SCALE:          return getScaleMat(scale);
    case TRANSFORMATION_ROTATE:         return getRotMat(Vector4d(), rotate, angle);
    case TRANSFORMATION_MATRIX:         return matrix;
    }

    return Matrix4x4::identity();
}