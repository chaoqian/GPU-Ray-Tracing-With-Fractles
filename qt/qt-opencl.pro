#
# vim:filetype=qmake sw=4 ts=4 expandtab nospell
#

BASEDIR = ..
TOPDIR = $$BASEDIR/..
UI_DIR = GeneratedFiles

NAME = qt-opencl

CONFIG += qt 
QT += opengl xml

#GLEW_DIR = c:/sdk/glew-1.9.0
GLEW_DIR = d:/glew/glew-1.9.0
OPECL_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v5.0"

win32 {
    DEFINES += NOMINMAX _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS TIXML_USE_STL 
    QMAKE_CXXFLAGS_WARN_ON += -W3 -wd4396 -wd4100 -wd4996
    QMAKE_LFLAGS += /INCREMENTAL:NO
}

LIBS += \
    -L$${GLEW_DIR}/lib \
    -L$${OPECL_DIR}/lib/x64 -lOpenCL \
    $$(NULL)


CONFIG(release, debug|release) {
TARGET = $$NAME
#    LIBS += \
#        -lopengl32 -lglu32 \
#        $$(NULL)
#    CONFIG += console
}
else {
TARGET = $${NAME}_d
#    LIBS += \
#        -lopengl32 -lglu32 \
#        $$(NULL)
    CONFIG += console
}

INCLUDEPATH += \
    $${BASEDIR} \
    $${UI_DIR} \
    $${GLEW_DIR}/include \
    $${OPECL_DIR}/include \
    $$(NULL)

include($${NAME}.pri)
