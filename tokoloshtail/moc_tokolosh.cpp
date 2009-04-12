/****************************************************************************
** Meta object code from reading C++ file 'tokolosh.h'
**
** Created: Sun Apr 12 17:03:34 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tokolosh.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tokolosh.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Tokolosh[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,
      30,   20,    9,    9, 0x05,
      52,    9,    9,    9, 0x05,
      65,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      88,   79,    9,    9, 0x0a,
     102,    9,    9,    9, 0x0a,
     109,    9,    9,    9, 0x0a,
     116,    9,    9,    9, 0x0a,
     124,    9,    9,    9, 0x0a,
     131,    9,    9,    9, 0x0a,
     138,    9,    9,    9, 0x0a,
     147,    9,    9,    9, 0x0a,
     162,    9,    9,    9, 0x0a,
     175,    9,    9,    9, 0x0a,
     191,    9,    9,    9, 0x0a,
     210,    9,  206,    9, 0x0a,
     222,    9,    9,    9, 0x0a,
     236,    9,  206,    9, 0x0a,
     244,    9,    9,    9, 0x0a,
     258,    9,    9,    9, 0x0a,
     283,  278,    9,    9, 0x0a,
     307,  278,    9,    9, 0x0a,
     334,    9,    9,    9, 0x0a,
     352,    9,    9,    9, 0x0a,
     395,  388,  375,    9, 0x0a,
     415,    9,  375,    9, 0x2a,
     432,    9,  375,    9, 0x0a,
     447,    9,  375,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Tokolosh[] = {
    "Tokolosh\0\0crashed()\0trackPath\0"
    "trackChanged(QString)\0repeat(bool)\0"
    "shuffle(bool)\0fileName\0load(QString)\0"
    "stop()\0play()\0pause()\0next()\0prev()\0"
    "volume()\0setVolume(int)\0toggleMute()\0"
    "toggleShuffle()\0toggleRepeat()\0int\0"
    "getStatus()\0syncClients()\0speed()\0"
    "setSpeed(int)\0clearLibraryPaths()\0"
    "path\0addLibraryPath(QString)\0"
    "removeLibraryPath(QString)\0populateLibrary()\0"
    "searchedLibraryPaths()\0QDBusVariant\0"
    "window\0playlistWindow(int)\0playlistWindow()\0"
    "libraryPaths()\0libraryContents()\0"
};

const QMetaObject Tokolosh::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Tokolosh,
      qt_meta_data_Tokolosh, 0 }
};

const QMetaObject *Tokolosh::metaObject() const
{
    return &staticMetaObject;
}

void *Tokolosh::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Tokolosh))
        return static_cast<void*>(const_cast< Tokolosh*>(this));
    return QObject::qt_metacast(_clname);
}

int Tokolosh::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: crashed(); break;
        case 1: trackChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: repeat((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: shuffle((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: load((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: stop(); break;
        case 6: play(); break;
        case 7: pause(); break;
        case 8: next(); break;
        case 9: prev(); break;
        case 10: volume(); break;
        case 11: setVolume((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: toggleMute(); break;
        case 13: toggleShuffle(); break;
        case 14: toggleRepeat(); break;
        case 15: { int _r = getStatus();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 16: syncClients(); break;
        case 17: { int _r = speed();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 18: setSpeed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: clearLibraryPaths(); break;
        case 20: addLibraryPath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 21: removeLibraryPath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: populateLibrary(); break;
        case 23: searchedLibraryPaths(); break;
        case 24: { QDBusVariant _r = playlistWindow((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QDBusVariant*>(_a[0]) = _r; }  break;
        case 25: { QDBusVariant _r = playlistWindow();
            if (_a[0]) *reinterpret_cast< QDBusVariant*>(_a[0]) = _r; }  break;
        case 26: { QDBusVariant _r = libraryPaths();
            if (_a[0]) *reinterpret_cast< QDBusVariant*>(_a[0]) = _r; }  break;
        case 27: { QDBusVariant _r = libraryContents();
            if (_a[0]) *reinterpret_cast< QDBusVariant*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 28;
    }
    return _id;
}

// SIGNAL 0
void Tokolosh::crashed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Tokolosh::trackChanged(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Tokolosh::repeat(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Tokolosh::shuffle(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
