/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[26];
    char stringdata[501];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 15), // "startDaqClicked"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 14), // "stopDaqClicked"
QT_MOC_LITERAL(4, 43, 25), // "pauseDaqMonitoringClicked"
QT_MOC_LITERAL(5, 69, 16), // "configDaqClicked"
QT_MOC_LITERAL(6, 86, 29), // "transmitStartDaqClickedSignal"
QT_MOC_LITERAL(7, 116, 28), // "transmitStopDaqClickedSignal"
QT_MOC_LITERAL(8, 145, 36), // "transmitPauseMonitoringClicke..."
QT_MOC_LITERAL(9, 182, 30), // "transmitConfigDaqClickedSignal"
QT_MOC_LITERAL(10, 213, 15), // "updateTabCanvas"
QT_MOC_LITERAL(11, 229, 5), // "index"
QT_MOC_LITERAL(12, 235, 20), // "getSelectedItem_slot"
QT_MOC_LITERAL(13, 256, 8), // "QString*"
QT_MOC_LITERAL(14, 265, 16), // "QAbstractButton*"
QT_MOC_LITERAL(15, 282, 6), // "button"
QT_MOC_LITERAL(16, 289, 27), // "openConfigurationPanel_slot"
QT_MOC_LITERAL(17, 317, 14), // "fillStatistics"
QT_MOC_LITERAL(18, 332, 20), // "fillStatisticsHistos"
QT_MOC_LITERAL(19, 353, 28), // "fillStatisticsAdvancedHistos"
QT_MOC_LITERAL(20, 382, 26), // "fillChargeStatisticsHistos"
QT_MOC_LITERAL(21, 409, 24), // "fillTimeStatisticsHistos"
QT_MOC_LITERAL(22, 434, 10), // "reset_slot"
QT_MOC_LITERAL(23, 445, 19), // "handleBufferedEvent"
QT_MOC_LITERAL(24, 465, 17), // "openPedestalsFile"
QT_MOC_LITERAL(25, 483, 17) // "loadPedestalsFile"

    },
    "MainWindow\0startDaqClicked\0\0stopDaqClicked\0"
    "pauseDaqMonitoringClicked\0configDaqClicked\0"
    "transmitStartDaqClickedSignal\0"
    "transmitStopDaqClickedSignal\0"
    "transmitPauseMonitoringClickedSignal\0"
    "transmitConfigDaqClickedSignal\0"
    "updateTabCanvas\0index\0getSelectedItem_slot\0"
    "QString*\0QAbstractButton*\0button\0"
    "openConfigurationPanel_slot\0fillStatistics\0"
    "fillStatisticsHistos\0fillStatisticsAdvancedHistos\0"
    "fillChargeStatisticsHistos\0"
    "fillTimeStatisticsHistos\0reset_slot\0"
    "handleBufferedEvent\0openPedestalsFile\0"
    "loadPedestalsFile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  114,    2, 0x06 /* Public */,
       3,    0,  115,    2, 0x06 /* Public */,
       4,    0,  116,    2, 0x06 /* Public */,
       5,    0,  117,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,  118,    2, 0x0a /* Public */,
       7,    0,  119,    2, 0x0a /* Public */,
       8,    0,  120,    2, 0x0a /* Public */,
       9,    0,  121,    2, 0x0a /* Public */,
      10,    1,  122,    2, 0x0a /* Public */,
      12,    1,  125,    2, 0x0a /* Public */,
      16,    0,  128,    2, 0x0a /* Public */,
      17,    0,  129,    2, 0x0a /* Public */,
      18,    0,  130,    2, 0x0a /* Public */,
      19,    0,  131,    2, 0x0a /* Public */,
      20,    0,  132,    2, 0x0a /* Public */,
      21,    0,  133,    2, 0x0a /* Public */,
      22,    0,  134,    2, 0x0a /* Public */,
      23,    0,  135,    2, 0x0a /* Public */,
      24,    0,  136,    2, 0x0a /* Public */,
      25,    0,  137,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    0x80000000 | 13, 0x80000000 | 14,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->startDaqClicked(); break;
        case 1: _t->stopDaqClicked(); break;
        case 2: _t->pauseDaqMonitoringClicked(); break;
        case 3: _t->configDaqClicked(); break;
        case 4: _t->transmitStartDaqClickedSignal(); break;
        case 5: _t->transmitStopDaqClickedSignal(); break;
        case 6: _t->transmitPauseMonitoringClickedSignal(); break;
        case 7: _t->transmitConfigDaqClickedSignal(); break;
        case 8: _t->updateTabCanvas((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: { QString* _r = _t->getSelectedItem_slot((*reinterpret_cast< QAbstractButton*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString**>(_a[0]) = _r; }  break;
        case 10: _t->openConfigurationPanel_slot(); break;
        case 11: _t->fillStatistics(); break;
        case 12: _t->fillStatisticsHistos(); break;
        case 13: _t->fillStatisticsAdvancedHistos(); break;
        case 14: _t->fillChargeStatisticsHistos(); break;
        case 15: _t->fillTimeStatisticsHistos(); break;
        case 16: _t->reset_slot(); break;
        case 17: _t->handleBufferedEvent(); break;
        case 18: _t->openPedestalsFile(); break;
        case 19: _t->loadPedestalsFile(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractButton* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MainWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MainWindow::startDaqClicked)) {
                *result = 0;
            }
        }
        {
            typedef void (MainWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MainWindow::stopDaqClicked)) {
                *result = 1;
            }
        }
        {
            typedef void (MainWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MainWindow::pauseDaqMonitoringClicked)) {
                *result = 2;
            }
        }
        {
            typedef void (MainWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MainWindow::configDaqClicked)) {
                *result = 3;
            }
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow.data,
      qt_meta_data_MainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::startDaqClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void MainWindow::stopDaqClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void MainWindow::pauseDaqMonitoringClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void MainWindow::configDaqClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
