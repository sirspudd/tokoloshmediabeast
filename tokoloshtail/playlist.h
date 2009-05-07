#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtCore>
#include <global.h>

struct PlaylistPrivate
{
    virtual ~PlaylistPrivate() {}
    QList<QHash<int, QVariant> > all; // should be possible to mmap this.
    QList<int> mapping;
    QRegExp filter;
    uint filterFields;
    Qt::SortOrder sortOrder;
    int sortField;
    struct {
        QHash<int, QVariant> cachedFields;
        QString cachedTrack;
    } mutable cache;
};

class Backend;
class Playlist : public QObject
{
    Q_OBJECT
public:
    ~Playlist() { delete d; }
public slots:
    bool validTrack(const QString &file) const; // static?
    bool addTrack(const QString &path);
    inline void addDirectory(const QString &path, bool recurse = false)
    { foreach(QString track, validTracks(path, recurse)) addTrack(track); }
    QStringList validTracks(const QString &path, bool recurse = false) const;

    int count() const;
    QByteArray fields(const QString &path, uint types = All) const;
    QByteArray fields(int track, uint types = All) const;

    QVariant field(int track, int field) const;
    QVariant field(const QString &file, int field) const;

    bool filter(const QByteArray &fields) const;

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

    QByteArray fields(int from, int size, uint types) const;
    bool remove(int track, int count = 1);

    bool move(int from, int to);
    bool swap(int from, int to);

    void sort(int f, Qt::SortOrder sortorder); // getter?
    void setFilter(const QRegExp &rx, uint fields = All);
    inline void setFilter(const QString &str, uint fields = All)
    { setFilter(QRegExp(QRegExp::escape(str)), fields); } // ### is this right?
    QRegExp filter() const;
    uint filterFields() const;
    //void startTransaction
    //void endTransaction // ### ???
    void requestAsyncTrackData(int track, int info);
    // maybe some identifier so clients that don't care don't need to know?
signals:
    void trackData(int track, int info, const QVariant &variant);
    void countChanged(int);
    void trackChanged(int);
    void tracksRemoved(int from, int count);
    void tracksChanged(int from, int count);
    void filterChanged(const QRegExp &rx, uint filterFields);
    void sortChanged(int field, Qt::SortOrder order);
protected:
    Playlist(PlaylistPrivate &dd, QObject *parent);
private:
    PlaylistPrivate *d;
    inline int dataIndex(int idx) const { return d->mapping.value(idx, idx); }
};


#endif
