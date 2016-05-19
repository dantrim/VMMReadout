/****************************************************************************
** Meta object code from reading C++ file 'pedestalsfileloader.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pedestalsfileloader.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pedestalsfileloader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_pedestalsFileLoader_t {
    QByteArrayData data[13];
    char stringdata[172];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_pedestalsFileLoader_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_pedestalsFileLoader_t qt_meta_stringdata_pedestalsFileLoader = {
    {
QT_MOC_LITERAL(0, 0, 19), // "pedestalsFileLoader"
QT_MOC_LITERAL(1, 20, 18), // "drawPedestalHistos"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 23), // "definePedestalsFileName"
QT_MOC_LITERAL(4, 64, 11), // "std::string"
QT_MOC_LITERAL(5, 76, 4), // "file"
QT_MOC_LITERAL(6, 81, 18), // "resetChipPedestals"
QT_MOC_LITERAL(7, 100, 17), // "fillChipPedestals"
QT_MOC_LITERAL(8, 118, 4), // "chip"
QT_MOC_LITERAL(9, 123, 11), // "chipChannel"
QT_MOC_LITERAL(10, 135, 7), // "pedMean"
QT_MOC_LITERAL(11, 143, 8), // "pedSigma"
QT_MOC_LITERAL(12, 152, 19) // "drawPedestalsHistos"

    },
    "pedestalsFileLoader\0drawPedestalHistos\0"
    "\0definePedestalsFileName\0std::string\0"
    "file\0resetChipPedestals\0fillChipPedestals\0"
    "chip\0chipChannel\0pedMean\0pedSigma\0"
    "drawPedestalsHistos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_pedestalsFileLoader[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   40,    2, 0x0a /* Public */,
       6,    0,   43,    2, 0x0a /* Public */,
       7,    4,   44,    2, 0x0a /* Public */,
      12,    0,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Float, QMetaType::Float,    8,    9,   10,   11,
    QMetaType::Void,

       0        // eod
};

void pedestalsFileLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        pedestalsFileLoader *_t = static_cast<pedestalsFileLoader *>(_o);
        switch (_id) {
        case 0: _t->drawPedestalHistos(); break;
        case 1: _t->definePedestalsFileName((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 2: _t->resetChipPedestals(); break;
        case 3: _t->fillChipPedestals((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3])),(*reinterpret_cast< float(*)>(_a[4]))); break;
        case 4: _t->drawPedestalsHistos(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (pedestalsFileLoader::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&pedestalsFileLoader::drawPedestalHistos)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject pedestalsFileLoader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_pedestalsFileLoader.data,
      qt_meta_data_pedestalsFileLoader,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *pedestalsFileLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *pedestalsFileLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_pedestalsFileLoader.stringdata))
        return static_cast<void*>(const_cast< pedestalsFileLoader*>(this));
    return QObject::qt_metacast(_clname);
}

int pedestalsFileLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void pedestalsFileLoader::drawPedestalHistos()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
