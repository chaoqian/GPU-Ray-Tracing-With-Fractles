/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.hpp'
**
** Created: Mon Dec 17 01:36:02 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MainWindow.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      35,   27,   11,   11, 0x0a,
      66,   27,   11,   11, 0x0a,
      97,   27,   11,   11, 0x0a,
     136,   27,   11,   11, 0x0a,
     181,  175,   11,   11, 0x0a,
     219,  175,   11,   11, 0x0a,
     257,  175,   11,   11, 0x0a,
     295,  175,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0updateCanvas()\0checked\0"
    "on_open_action_triggered(bool)\0"
    "on_exit_action_triggered(bool)\0"
    "on_raytrace_cpu_action_triggered(bool)\0"
    "on_raytrace_gpu_action_triggered(bool)\0"
    "value\0on_fractal_x_slider_valueChanged(int)\0"
    "on_fractal_y_slider_valueChanged(int)\0"
    "on_fractal_z_slider_valueChanged(int)\0"
    "on_fractal_w_slider_valueChanged(int)\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->updateCanvas(); break;
        case 1: _t->on_open_action_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->on_exit_action_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->on_raytrace_cpu_action_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->on_raytrace_gpu_action_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->on_fractal_x_slider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->on_fractal_y_slider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->on_fractal_z_slider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->on_fractal_w_slider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "Ui::MainWindow"))
        return static_cast< Ui::MainWindow*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
