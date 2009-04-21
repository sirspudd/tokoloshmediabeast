#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtCore>

class Playlist : public QObject
{
    Q_OBJECT
public:
    enum Field {
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
    Playlist(QObject *parent = 0);
    bool validSong(const QString &file) const; // static?
    inline int addFile(const QString &path) { return insertSong(count(), path); }
    inline void addDirectory(const QString &path, bool recurse = false) { foreach(QString song, validSongs(path, recurse)) addFile(song); }
    QStringList validSongs(const QString &path, bool recurse = false) const;
    int count() const;
    int insertSong(int index, const QString &path);
    QHash<Field, QVariant> fields(const QString &path, uint types = All) const;
    QHash<Field, QVariant> fields(int track, uint types = All) const;

    QVariant field(int track, Field field) const;
    QVariant field(const QString &file, Field field) const;

    bool filter(int index) const;
    bool filter(const QString &file) const;
    bool filter(const QHash<Field, QVariant> &fields) const;

    inline QString fileName(int track) const { return field(track, FileName).toString(); }
    inline QString filePath(int track) const { return field(track, FilePath).toString(); }
    inline QString fileDirectory(int track) const { return field(track, FileDirectory).toString(); }
    inline QString trackName(int track) const { return field(track, TrackName).toString(); }
    inline int trackLength(int track) const { return field(track, TrackName).toInt(); }
    inline QString artist(int track) const { return field(track, Artist).toString(); }
    inline QString album(int track) const { return field(track, Album).toString(); }
    inline int year(int track) const { return field(track, TrackName).toInt(); }
    inline int trackNumber(int track) const { return field(track, TrackNumber).toInt(); }
    inline QString genre(int track) const { return field(track, Genre).toString(); }

    inline QString fileName(const QString &path) const { return field(path, FileName).toString(); }
    inline QString filePath(const QString &path) const { return field(path, FilePath).toString(); }
    inline QString fileDirectory(const QString &path) const { return field(path, FileDirectory).toString(); }
    inline QString trackName(const QString &path) const { return field(path, TrackName).toString(); }
    inline int trackLength(const QString &path) const { return field(path, TrackName).toInt(); }
    inline QString artist(const QString &path) const { return field(path, Artist).toString(); }
    inline QString album(const QString &path) const { return field(path, Album).toString(); }
    inline int year(const QString &path) const { return field(path, TrackName).toInt(); }
    inline int trackNumber(const QString &path) const { return field(path, TrackNumber).toInt(); }
    inline QString genre(const QString &path) const { return field(path, Genre).toString(); }

    QList<QHash<Field, QVariant> > fields(int from, int size, uint types) const;
    bool remove(int track, int count = 1);

    bool move(int from, int to);
    bool swap(int from, int to);

    void sort(Field f, Qt::SortOrder sortorder); // getter?
    void setFilter(const QRegExp &rx, uint fields = All);
    inline void setFilter(const QString &str, uint fields = All)
    { setFilter(QRegExp(QRegExp::escape(str)), fields); } // ### is this right?
    QRegExp filter() const;
    uint filterFields() const;
    //void startTransaction
    //void endTransaction // ### ???
signals:
    void countChanged(int);
    void trackChanged(int);
    void trackInserted(int);
    void tracksRemoved(int from, int count);
    void tracksChanged(int from, int count);
private:
    inline int dataIndex(int idx) const { return d.mapping.value(idx, idx); }
    struct Private {
        QList<QHash<Field, QVariant> > all; // should be possible to mmap this.
        QVector<int> mapping;
        QRegExp filter;
        uint filterFields;
        Qt::SortOrder sortOrder;
        Field sortField;
    } d;
};


#endif
