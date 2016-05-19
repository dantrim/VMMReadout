/****************************************************************************
** Meta object code from reading C++ file 'mmDaqSettingsWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mmDaqSettingsWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mmDaqSettingsWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_mmDaqSettingsWindow_t {
    QByteArrayData data[15];
    char stringdata[307];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_mmDaqSettingsWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_mmDaqSettingsWindow_t qt_meta_stringdata_mmDaqSettingsWindow = {
    {
QT_MOC_LITERAL(0, 0, 19), // "mmDaqSettingsWindow"
QT_MOC_LITERAL(1, 20, 22), // "browsePedestalsClicked"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 20), // "loadPedestalsClicked"
QT_MOC_LITERAL(4, 65, 19), // "pedestalsFilePathIs"
QT_MOC_LITERAL(5, 85, 19), // "pedestalsFileNameIs"
QT_MOC_LITERAL(6, 105, 21), // "rebrowseConfigClicked"
QT_MOC_LITERAL(7, 127, 19), // "reloadConfigClicked"
QT_MOC_LITERAL(8, 147, 18), // "reconfigFilePathIs"
QT_MOC_LITERAL(9, 166, 18), // "reconfigFileNameIs"
QT_MOC_LITERAL(10, 185, 17), // "loadPedestalsFile"
QT_MOC_LITERAL(11, 203, 19), // "searchForConfigFile"
QT_MOC_LITERAL(12, 223, 28), // "transmitConfigFileNameSignal"
QT_MOC_LITERAL(13, 252, 22), // "searchForPedestalsFile"
QT_MOC_LITERAL(14, 275, 31) // "transmitPedestalsFileNameSignal"

    },
    "mmDaqSettingsWindow\0browsePedestalsClicked\0"
    "\0loadPedestalsClicked\0pedestalsFilePathIs\0"
    "pedestalsFileNameIs\0rebrowseConfigClicked\0"
    "reloadConfigClicked\0reconfigFilePathIs\0"
    "reconfigFileNameIs\0loadPedestalsFile\0"
    "searchForConfigFile\0transmitConfigFileNameSignal\0"
    "searchForPedestalsFile\0"
    "transmitPedestalsFileNameSignal"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_mmDaqSettingsWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    0,   80,    2, 0x06 /* Public */,
       4,    1,   81,    2, 0x06 /* Public */,
       5,    1,   84,    2, 0x06 /* Public */,
       6,    0,   87,    2, 0x06 /* Public */,
       7,    0,   88,    2, 0x06 /* Public */,
       8,    1,   89,    2, 0x06 /* Public */,
       9,    1,   92,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   95,    2, 0x0a /* Public */,
      11,    0,   96,    2, 0x0a /* Public */,
      12,    0,   97,    2, 0x0a /* Public */,
      13,    0,   98,    2, 0x0a /* Public */,
      14,    0,   99,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void mmDaqSettingsWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        mmDaqSettingsWindow *_t = static_cast<mmDaqSettingsWindow *>(_o);
        switch (_id) {
        case 0: _t->browsePedestalsClicked(); break;
        case 1: _t->loadPedestalsClicked(); break;
        case 2: _t->pedestalsFilePathIs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->pedestalsFileNameIs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->rebrowseConfigClicked(); break;
        case 5: _t->reloadConfigClicked(); break;
        case 6: _t->reconfigFilePathIs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->reconfigFileNameIs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->loadPedestalsFile(); break;
        case 9: _t->searchForConfigFile(); break;
        case 10: _t->transmitConfigFileNameSignal(); break;
        case 11: _t->searchForPedestalsFile(); break;
        case 12: _t->transmitPedestalsFileNameSignal(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (mmDaqSettingsWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::browsePedestalsClicked)) {
                *result = 0;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::loadPedestalsClicked)) {
                *result = 1;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::pedestalsFilePathIs)) {
                *result = 2;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::pedestalsFileNameIs)) {
                *result = 3;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::rebrowseConfigClicked)) {
                *result = 4;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::reloadConfigClicked)) {
                *result = 5;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::reconfigFilePathIs)) {
                *result = 6;
            }
        }
        {
            typedef void (mmDaqSettingsWindow::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&mmDaqSettingsWindow::reconfigFileNameIs)) {
                *result = 7;
            }
        }
    }
}

const QMetaObject mmDaqSettingsWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_mmDaqSettingsWindow.data,
      qt_meta_data_mmDaqSettingsWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *mmDaqSettingsWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *mmDaqSettingsWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_mmDaqSettingsWindow.stringdata))
        return static_cast<void*>(const_cast< mmDaqSettingsWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int mmDaqSettingsWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void mmDaqSettingsWindow::browsePedestalsClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void mmDaqSettingsWindow::loadPedestalsClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void mmDaqSettingsWindow::pedestalsFilePathIs(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void mmDaqSettingsWindow::pedestalsFileNameIs(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void mmDaqSettingsWindow::rebrowseConfigClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}

// SIGNAL 5
void mmDaqSettingsWindow::reloadConfigClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 5, Q_NULLPTR);
}

// SIGNAL 6
void mmDaqSettingsWindow::reconfigFilePathIs(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void mmDaqSettingsWindow::reconfigFileNameIs(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_END_MOC_NAMESPACE
