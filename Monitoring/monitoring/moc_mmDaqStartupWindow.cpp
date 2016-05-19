/****************************************************************************
** Meta object code from reading C++ file 'mmDaqStartupWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mmDaqStartupWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mmDaqStartupWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_online__display__mmDaqStartupWindow_t {
    QByteArrayData data[8];
    char stringdata[158];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_online__display__mmDaqStartupWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_online__display__mmDaqStartupWindow_t qt_meta_stringdata_online__display__mmDaqStartupWindow = {
    {
QT_MOC_LITERAL(0, 0, 35), // "online::display::mmDaqStartup..."
QT_MOC_LITERAL(1, 36, 16), // "configFilePathIs"
QT_MOC_LITERAL(2, 53, 0), // ""
QT_MOC_LITERAL(3, 54, 16), // "configFileNameIs"
QT_MOC_LITERAL(4, 71, 24), // "setUpConfigFileSelection"
QT_MOC_LITERAL(5, 96, 12), // "QVBoxLayout*"
QT_MOC_LITERAL(6, 109, 19), // "searchForConfigFile"
QT_MOC_LITERAL(7, 129, 28) // "transmitConfigFileNameSignal"

    },
    "online::display::mmDaqStartupWindow\0"
    "configFilePathIs\0\0configFileNameIs\0"
    "setUpConfigFileSelection\0QVBoxLayout*\0"
    "searchForConfigFile\0transmitConfigFileNameSignal"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_online__display__mmDaqStartupWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       3,    1,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   45,    2, 0x0a /* Public */,
       6,    0,   46,    2, 0x0a /* Public */,
       7,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    0x80000000 | 5,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void online::display::mmDaqStartupWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        mmDaqStartupWindow *_t = static_cast<mmDaqStartupWindow *>(_o);
        switch (_id) {
        case 0: _t->configFilePathIs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->configFileNameIs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: { QVBoxLayout* _r = _t->setUpConfigFileSelection();
            if (_a[0]) *reinterpret_cast< QVBoxLayout**>(_a[0]) = _r; }  break;
        case 3: _t->searchForConfigFile(); break;
        case 4: _t->transmitConfigFileNameSignal(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (mmDaqStartupWindow::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqStartupWindow::configFilePathIs)) {
                *result = 0;
            }
        }
        {
            typedef void (mmDaqStartupWindow::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqStartupWindow::configFileNameIs)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject online::display::mmDaqStartupWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_online__display__mmDaqStartupWindow.data,
      qt_meta_data_online__display__mmDaqStartupWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *online::display::mmDaqStartupWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *online::display::mmDaqStartupWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_online__display__mmDaqStartupWindow.stringdata))
        return static_cast<void*>(const_cast< mmDaqStartupWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int online::display::mmDaqStartupWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void online::display::mmDaqStartupWindow::configFilePathIs(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void online::display::mmDaqStartupWindow::configFileNameIs(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
