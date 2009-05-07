#ifndef GLOBAL_H
#define GLOBAL_H

enum TrackInfo {
    None = 0x000,
    FileName = 0x001,
    FilePath = 0x002,
    FileDirectory = 0x004,
    TrackName = 0x008,
    TrackLength = 0x010,
    Artist = 0x020,
    Album = 0x040,
    Year = 0x080,
    Genre = 0x100,
    TrackNumber = 0x200,
    All = 0xfff
};

template <typename T> static inline T fromByteArray(const QByteArray &ba)
{
    QDataStream ds(ba);
    T t;
    ds >> t;
    return t;
}

template <typename T> static inline QByteArray toByteArray(const T &t)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << t;
    return ba;
}

#endif
