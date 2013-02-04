
#include <GL/glew.h>
#include "GLWidget.hpp"

static QGLFormat glFormat;

GLWidget::GLWidget(QWidget * parent) :
QGLWidget((glFormat.setSwapInterval(0), glFormat), parent),
    texture_id(GL_INVALID_VALUE)
{
}

GLWidget::~GLWidget()
{
    deleteTexture(texture_id);
}

void GLWidget::initializeGL()
{
    // Set up the rendering context, define display lists etc.
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);

    glEnable(GL_TEXTURE_2D);

    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void GLWidget::resizeGL(int w, int h)
{
    // setup viewport, projection etc.:
    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    //glOrtho(0.0, 1.0, -1.0, 0.0, 0.0, 1.0);
    //glScalef(1.f, -1.f, 1.f); //flip the y axis

    // make a new texture name
    deleteTexture(texture_id);
    glGenTextures(1, &texture_id);

    //resize the underlying texture
    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(Qt::black);
    set_image(image);
}

void GLWidget::set_image(QImage const& image)
{
    if (texture_id==GL_INVALID_VALUE || image.isNull())
    {
        return;
    }

    //bind
    glBindTexture(GL_TEXTURE_2D, texture_id);

    //copy
    QImage gl_compatible_image = GLWidget::convertToGLFormat(image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_compatible_image.width(), gl_compatible_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gl_compatible_image.bits());

    //texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    //update current view
    updateGL();
}

void GLWidget::paintGL()
{
    // select our current texture
    glBindTexture(GL_TEXTURE_2D, texture_id);

    //draw
    glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2d(0.0,0.0);
    glTexCoord2d(1.0,0.0); glVertex2d(1.0,0.0);
    glTexCoord2d(1.0,1.0); glVertex2d(1.0,1.0);
    glTexCoord2d(0.0,1.0); glVertex2d(0.0,1.0);
    glEnd();

    //unbind
    glBindTexture(GL_TEXTURE_2D, 0);
}