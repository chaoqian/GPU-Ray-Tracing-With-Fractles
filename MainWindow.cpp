
#include "MainWindow.hpp"
#include <Scene.hpp>
#include <QLabel>
#include <QFileDialog>
#include <QTimer>
#include <QMouseEvent>
#include <QList>
#include "lib/CS123XmlSceneParser.h"

#include <iostream>
using std::cout;
using std::endl;


MainWindow::MainWindow(QWidget * parent, Qt::WindowFlags flags) :
  QMainWindow(parent, flags),
  raytrace_cpu(),
  raytrace_gpu(),
  frames(0),
  render_time(),
  fps_label(new QLabel(this)),
  m_scene(NULL),
  m_camera()
{
    setupUi(this);
    canvas_glwidget->installEventFilter(this);

    statusBar()->addWidget(fps_label);
    raytrace_gpu_action->setChecked(true);

    if (!raytrace_gpu.init(canvas_glwidget))
    {   //error in init OpenCL
        show();
        QMessageBox::critical(this, "Raycast GPU Error", raytrace_gpu.get_error_log());
        return;
    }

    //default camera
    CS123SceneCameraData camera_data;
    camera_data.pos  = Vector4d(5, 5, 5, 1);
    camera_data.up   = Vector4d(0, 1, 0, 0);
    camera_data.look = Vector4d(-1, -1, -1, 0);
    camera_data.heightAngle = 45;
    camera_data.aspectRatio = static_cast<double>(canvas_glwidget->width())/canvas_glwidget->height();
    m_camera.orientLook(camera_data.pos, camera_data.look, camera_data.up);
    m_camera.setHeightAngle(camera_data.heightAngle);
    m_camera.setAspectRatio(camera_data.aspectRatio);

    m_captureMouse = true; // can not use mouse

    //julia slider
    fractal_x_slider->setMinimum(-25); fractal_x_slider->setMaximum(25); fractal_x_slider->setValue(0);
    fractal_y_slider->setMinimum(-25); fractal_y_slider->setMaximum(25); fractal_y_slider->setValue(0);
    fractal_z_slider->setMinimum(-25); fractal_z_slider->setMaximum(25); fractal_z_slider->setValue(0);
    fractal_w_slider->setMinimum(-25); fractal_w_slider->setMaximum(25); fractal_w_slider->setValue(0);

    render_time.start();
    QTimer::singleShot(0, this, SLOT(updateCanvas()));
}

MainWindow::~MainWindow()
{
    if (m_scene)
    {
        delete m_scene;
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    bool rv = QObject::eventFilter(obj, e);

    if (e->type() == QEvent::Resize)
    {
        double aspectRatio = static_cast<double>(canvas_glwidget->width())/canvas_glwidget->height();
        m_camera.setAspectRatio(aspectRatio);
        render();
    }

    if( e->type() == QEvent::MouseButtonPress && obj == canvas_glwidget)
    {
        if(m_captureMouse)
        {
        QMouseEvent *mouseevent = (QMouseEvent*)(e);
        if(mouseevent->buttons()&Qt::LeftButton) // handle left click
        {
           m_preMousePos.x = mouseevent->x();
           m_preMousePos.y = mouseevent->y();

        }
        }
    }

    if( e->type() == QEvent::MouseMove && obj == canvas_glwidget)
    {
        if(m_captureMouse)
        {
        QMouseEvent *mouseevent = (QMouseEvent*)(e);
        Vector2i pos(mouseevent->x(), mouseevent->y());
        if(mouseevent->buttons()&Qt::LeftButton)
        {
            m_camera.rotateV((pos.x - m_preMousePos.x)*0.1);
            m_camera.rotateU(-(pos.y - m_preMousePos.y)*0.1);

        }
        m_preMousePos = pos;
        }

    }

    if(e->type() == QEvent::Wheel && obj == canvas_glwidget)
    {
        if(m_captureMouse)
        {
            QWheelEvent *w = (QWheelEvent*)(e);
            if(w->orientation() == Qt::Vertical ) // if is verticle orientation
            {
                m_camera.translate(-(m_camera.getLook().getNormalized())* w->delta()*0.005f);
            }
        }
    }

    return rv;
}

void MainWindow::render(void)
{
    if (raytrace_gpu_action->isChecked())
    {
        raytrace_gpu.render(m_camera, canvas_glwidget->get_texture());
        canvas_glwidget->updateGL();
    }
    else
    {
        QImage image(canvas_glwidget->width(), canvas_glwidget->height(), QImage::Format_ARGB32);
        raytrace_cpu.render(m_scene, m_camera, image);
        canvas_glwidget->set_image(image);
    }

    frames++;
}

void MainWindow::updateCanvas()
{
    static QTime start = QTime::currentTime();

    /* rotating the camera */
//    double degree_per_second = 10;
//    m_camera.rotate(Vector4d(0,0,0,1), Vector4d(0,1,0,0), degree_per_second * render_time.elapsed() * 0.001); //! rotate around the V

    /* rotating the object: self rotating */
    /*if (m_scene)
    {
        QList<Scene::ScenePrimitive> & t = m_scene->getPrimitives();
        for( int i = 0; i < t.size(); i++ ) // for every object in the scene
        {
            t[i].T *= getRotMat(Vector4(0,0,0,1), Vector4(0,1,0,0), 0.1);
        }
        raytrace_gpu.setScene(m_scene);
    }*/

    render();

    if (frames>=10 && render_time.elapsed()>1000)
    {
        int elapsed = render_time.elapsed();
        bool use_gpu = raytrace_gpu_action->isChecked();
        double fps = frames*1000.0/elapsed;
        fps_label->setText(QString::number(fps, 'g', 3)+" fps "+QString("[%1]").arg((use_gpu?"GPU: "+raytrace_gpu.get_description():"CPU")));
        frames = 0;
        render_time.restart();
    }

    QTimer::singleShot(0, this, SLOT(updateCanvas()));/* time out as soon as events all been processed */
}

void MainWindow::on_open_action_triggered(bool checked)
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Scene", "", "Scene Files (*.xml)");
    if (!fileName.isNull())
    {
        CS123XmlSceneParser parser(qPrintable(fileName));
        if (parser.parse())
        {
            std::cerr << "Scene " << qPrintable(fileName) << std::endl;
            if (m_scene)
            {
                delete m_scene;
            }
            m_scene = new Scene;
            Scene::parse(m_scene, &parser);

            // Set the camera for the new scene
            CS123SceneCameraData camera_data;
            if (parser.getCameraData(camera_data))
            {
                camera_data.pos.data[3] = 1;
                camera_data.look.data[3] = 0;
                camera_data.up.data[3] = 0;

                m_camera.orientLook(camera_data.pos, camera_data.look, camera_data.up);
                m_camera.setHeightAngle(camera_data.heightAngle);
            }

            raytrace_gpu.setScene(m_scene);
        }
        else
        {
            QMessageBox::critical(this, "Error", "Could not load scene \"" + fileName + "\"");
        }
    }
}

void MainWindow::on_raytrace_cpu_action_triggered(bool checked)
{
    if (checked)
    {
        raytrace_gpu_action->blockSignals(true);
        raytrace_gpu_action->setChecked(false);
        raytrace_gpu_action->blockSignals(false);
    }
    else if (!checked && !raytrace_gpu_action->isChecked())
    {
        raytrace_cpu_action->setChecked(true);
    }
}

void MainWindow::on_raytrace_gpu_action_triggered(bool checked)
{
    if (checked)
    {
        raytrace_cpu_action->blockSignals(true);
        raytrace_cpu_action->setChecked(false);
        raytrace_cpu_action->blockSignals(false);
    }
    else if (!checked && !raytrace_cpu_action->isChecked())
    {
        raytrace_gpu_action->setChecked(true);
    }
}

void MainWindow::on_fractal_x_slider_valueChanged(int value)
{
    raytrace_gpu.setJuliaMuX(value);
}

void MainWindow::on_fractal_y_slider_valueChanged(int value)
{
    raytrace_gpu.setJuliaMuY(value);
}

void MainWindow::on_fractal_z_slider_valueChanged(int value)
{
    raytrace_gpu.setJuliaMuZ(value);
}

void MainWindow::on_fractal_w_slider_valueChanged(int value)
{
    raytrace_gpu.setJuliaMuW(value);
}