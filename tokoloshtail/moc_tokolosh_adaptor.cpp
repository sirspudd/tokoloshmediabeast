/****************************************************************************
** Meta object code from reading C++ file 'tokolosh_adaptor.h'
**
** Created: Sun Apr 12 17:03:34 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tokolosh_adaptor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tokolosh_adaptor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MediaAdaptor[] = {

 // content:
       2,       // revision
       0,       // classname
       2,   12, // classinfo
      19,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // classinfo: key, value
      57,   13,
    1145,   73,

 // signals: signature, parameters, type, tag, flags
    1166, 1165, 1165, 1165, 0x05,
    1181, 1176, 1165, 1165, 0x05,

 // slots: signature, parameters, type, tag, flags
    1203, 1165, 1165, 1165, 0x0a,
    1223, 1165, 1165, 1165, 0x0a,
    1235, 1176, 1165, 1165, 0x0a,
    1249, 1165, 1165, 1165, 0x0a,
    1256, 1165, 1165, 1165, 0x0a,
    1264, 1165, 1165, 1165, 0x0a,
    1271, 1165, 1165, 1165, 0x0a,
    1283, 1165, 1278, 1165, 0x0a,
    1292, 1165, 1165, 1165, 0x0a,
    1319, 1312, 1165, 1165, 0x0a,
    1334, 1165, 1278, 1165, 0x0a,
    1344, 1165, 1165, 1165, 0x0a,
    1351, 1165, 1165, 1165, 0x0a,
    1365, 1165, 1165, 1165, 0x0a,
    1378, 1165, 1165, 1165, 0x0a,
    1393, 1165, 1165, 1165, 0x0a,
    1413, 1165, 1409, 1165, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MediaAdaptor[] = {
    "MediaAdaptor\0com.TokoloshXineBackend.TokoloshMediaPlayer\0"
    "D-Bus Interface\0"
    "  <interface name=\"com.TokoloshXineBackend.TokoloshMediaPlayer\" >\n "
    "   <method name=\"play\" />\n    <method name=\"stop\" />\n    <method"
    " name=\"prev\" />\n    <method name=\"next\" />\n    <method name=\"pa"
    "use\" />\n    <method name=\"clearLibraryPaths\" />\n    <method name="
    "\"repopulateLibrary\" />\n    <method name=\"toggleMute\" />\n    <met"
    "hod name=\"toggleShuffle\" />\n    <method name=\"toggleRepeat\" />\n "
    "   <method name=\"syncClients\" />\n    <method name=\"dumpFiles\" />\n"
    "    <signal name=\"crashed\" />\n    <method name=\"shuffle\" >\n     "
    " <arg direction=\"out\" type=\"b\" name=\"enabled\" />\n    </method>\n"
    "    <method name=\"repeat\" >\n      <arg direction=\"out\" type=\"b\""
    " name=\"enabled\" />\n    </method>\n    <method name=\"volume\" >\n  "
    "    <arg direction=\"out\" type=\"i\" name=\"volume\" />\n    </method"
    ">\n    <method name=\"setVolume\" >\n      <arg direction=\"in\" type="
    "\"i\" name=\"volume\" />\n    </method>\n    <method name=\"load\" >\n"
    "      <arg direction=\"in\" type=\"s\" name=\"path\" />\n    </method>"
    "\n    <signal name=\"trackChanged\" >\n      <arg direction=\"out\" ty"
    "pe=\"s\" name=\"path\" />\n    </signal>\n  </interface>\n\0"
    "D-Bus Introspection\0\0crashed()\0path\0"
    "trackChanged(QString)\0clearLibraryPaths()\0"
    "dumpFiles()\0load(QString)\0next()\0"
    "pause()\0play()\0prev()\0bool\0repeat()\0"
    "repopulateLibrary()\0volume\0setVolume(int)\0"
    "shuffle()\0stop()\0syncClients()\0"
    "toggleMute()\0toggleRepeat()\0toggleShuffle()\0"
    "int\0volume()\0"
};

const QMetaObject MediaAdaptor::staticMetaObject = {
    { &QDBusAbstractAdaptor::staticMetaObject, qt_meta_stringdata_MediaAdaptor,
      qt_meta_data_MediaAdaptor, 0 }
};

const QMetaObject *MediaAdaptor::metaObject() const
{
    return &staticMetaObject;
}

void *MediaAdaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MediaAdaptor))
        return static_cast<void*>(const_cast< MediaAdaptor*>(this));
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int MediaAdaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: crashed(); break;
        case 1: trackChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: clearLibraryPaths(); break;
        case 3: dumpFiles(); break;
        case 4: load((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: next(); break;
        case 6: pause(); break;
        case 7: play(); break;
        case 8: prev(); break;
        case 9: { bool _r = repeat();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 10: repopulateLibrary(); break;
        case 11: setVolume((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: { bool _r = shuffle();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 13: stop(); break;
        case 14: syncClients(); break;
        case 15: toggleMute(); break;
        case 16: toggleRepeat(); break;
        case 17: toggleShuffle(); break;
        case 18: { int _r = volume();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void MediaAdaptor::crashed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MediaAdaptor::trackChanged(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
