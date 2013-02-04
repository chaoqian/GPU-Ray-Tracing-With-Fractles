
#ifndef __GLWIDGET_HPP__
#define __GLWIDGET_HPP__

#include <QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT;
public:
    GLWidget(QWidget * parent = 0);
    ~GLWidget();

    void set_image(QImage const& image);
    inline GLuint get_texture(void) const { return texture_id; }

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    GLuint texture_id;
};

#endif //__GLWIDGET_HPP__