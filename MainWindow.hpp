
#ifndef __MAINWINDOW_HPP__
#define __MAINWINDOW_HPP__

#include <QMainWindow>
#include <QTime>
#include "RayTraceCPU.hpp"
#include "RayTraceGPU.hpp"
#include "Scene.hpp"
#include "camera/CamtransCamera.h"

#include "ui_MainWindow.h"

class QPaintEvent;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT;
public:
    MainWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~MainWindow();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *e);
    void render(void);

public slots:
    void updateCanvas();
    void on_open_action_triggered(bool checked);
    void on_exit_action_triggered(bool checked) {close();}
    void on_raytrace_cpu_action_triggered(bool checked);
    void on_raytrace_gpu_action_triggered(bool checked);

    void on_fractal_x_slider_valueChanged(int value);
    void on_fractal_y_slider_valueChanged(int value);
    void on_fractal_z_slider_valueChanged(int value);
    void on_fractal_w_slider_valueChanged(int value);

private:
    RayTraceCPU raytrace_cpu;
    RayTraceGPU raytrace_gpu;
    int frames;
    QTime render_time;
    QLabel * fps_label;

    Scene * m_scene;
    CamtransCamera m_camera;

    //! if we can allowed to capture mouse
    bool m_captureMouse;
    Vector2i m_preMousePos;

};

#endif //__MAINWINDOW_HPP__
