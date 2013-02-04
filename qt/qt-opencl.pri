#
# vim:filetype=qmake sw=4 ts=4 expandtab nospell
#

HEADERS +=  \
            $${BASEDIR}/MainWindow.hpp \
            $${BASEDIR}/GLWidget.hpp \
            $${BASEDIR}/RayTraceCPU.hpp \
            $${BASEDIR}/RayTraceGPU.hpp \
            $${BASEDIR}/Scene.hpp \
            $${BASEDIR}/ShapeCube.hpp \
            $${BASEDIR}/ShapeSphere.hpp \
            $${BASEDIR}/ShapeCylinder.hpp \
            $${BASEDIR}/ShapeCone.hpp \
            $${BASEDIR}/ShapeMandelbulb.hpp \
            $${BASEDIR}/lib/CS123Common.h \
            $${BASEDIR}/lib/CS123SceneData.h \
            $${BASEDIR}/lib/CS123ISceneParser.h \
            $${BASEDIR}/lib/CS123XmlSceneParser.h \
            $${BASEDIR}/math/CS123Algebra.h \
            $${BASEDIR}/camera/Camera.h \
            $${BASEDIR}/camera/CamtransCamera.h \
            $$(NULL)

SOURCES +=  \
            $${BASEDIR}/main.cpp \
            $${BASEDIR}/MainWindow.cpp \
            $${BASEDIR}/GLWidget.cpp \
            $${BASEDIR}/RayTraceCPU.cpp \
            $${BASEDIR}/RayTraceGPU.cpp \
            $${BASEDIR}/Scene.cpp \
            $${BASEDIR}/ShapeCube.cpp \
            $${BASEDIR}/ShapeSphere.cpp \
            $${BASEDIR}/ShapeCylinder.cpp \
            $${BASEDIR}/ShapeCone.cpp \
            $${BASEDIR}/ShapeMandelbulb.cpp \
            $${BASEDIR}/lib/CS123SceneData.cpp \
            $${BASEDIR}/lib/CS123XmlSceneParser.cpp \
            $${BASEDIR}/math/CS123Matrix.cpp \
            $${BASEDIR}/camera/CamtransCamera.cpp \
            $$(NULL)

FORMS +=    \
            $${BASEDIR}/forms/MainWindow.ui \
            $$(NULL)

RESOURCES += $${BASEDIR}/gpu/opencl.qrc
