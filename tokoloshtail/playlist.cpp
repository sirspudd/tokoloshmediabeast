#include "playlist.h"
#include <xine.h>

#define VERIFY_INDEX(idx) { Q_ASSERT_X(idx >= 0 && idx < count(), \
                                       qPrintable(QString("Index out of range %1 (0-%2)").arg(idx).arg(count())), \
                                       __FUNCTION__); }

Playlist::Playlist(QObject *parent)
    : QObject(parent)
{
    d.filterFields = All;
}

bool Playlist::validSong(const QString &file) const
{
    // need to ask xine
    return true;
}

static inline QFileInfo resolveSymlink(const QString &file)
{
    QFileInfo fi(file);
    QStringList paths;
    forever {
        if (paths.contains(fi.absoluteFilePath())) {
            qWarning("%s is a recursive symlink", qPrintable(paths.join(" => ")));
            return QFileInfo();
        }
        paths.append(fi.absoluteFilePath());
        if (!fi.isSymLink()) {
            break;
        }
        fi = fi.symLinkTarget();
    }
    return fi;
}


QStringList Playlist::validSongs(const QString &path, bool recurse) const
{
    QDir dir(path);
    if (!dir.exists())
        return QStringList();
    QStringList ret;
    foreach(QString string, dir.entryList(recurse ? QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot : QDir::Files)) {
        const QFileInfo fi = resolveSymlink(path + QLatin1Char('/') + string);
        const QString abs = fi.absoluteFilePath();
        if (fi.isDir()) {
            Q_ASSERT(recurse);
            ret += validSongs(abs, recurse);
        } else if (validSong(abs)) {
            ret.append(abs);
        }
    }
    return ret;
}

int Playlist::count() const
{
    return d.mapping.isEmpty() ? d.all.size() : d.mapping.size();
}

int Playlist::insertSong(int index, const QString &path)
{

    QHash<Field, QVariant> f = fields(path);
    if (f.isEmpty())
        return -1;

    if (index < 0)
        index = count();
    Q_ASSERT(index <= count());
    if (!d.mapping.isEmpty()) {
        qDebug("%s %d: if (!d.mapping.isEmpty()) {", __FILE__, __LINE__);
    } else {
        d.all.insert(index, f);
    }
    emit trackInserted(index);
    if (index + 1 < count()) {
        emit tracksChanged(index, count() - index - 1); // ### is this right?
    }
    return index;
}

QHash<Playlist::Field, QVariant> Playlist::fields(const QString &path, uint fields) const
{
    QHash<Field, QVariant> ret;
    if (!validSong(path))
        return ret;
    QFileInfo fi(path);
    if (fields & FileName) {
        ret[FileName] = fi.fileName();
        fields &= ~FileName;
    }
    if (fields & FilePath) {
        ret[FilePath] = fi.absoluteFilePath();
        fields &= ~FilePath;
    }

    if (fields & FileDirectory) {
        ret[FilePath] = fi.path();
        fields &= ~FileDirectory;
    }

    if (fields) {
        // need to ask xine for stuff
    }
    return ret;
}

QHash<Playlist::Field, QVariant> Playlist::fields(int track, uint fields) const
{
    VERIFY_INDEX(track);
    const int idx = dataIndex(track);
    QHash<Field, QVariant> f = d.all.value(idx);
    if (fields != All) {
        for (QHash<Field, QVariant>::iterator it = f.begin(); it != f.end(); ++it) {
            if (!(fields & it.key()))
                f.erase(it);
        }
    }
    return f;
}

QVariant Playlist::field(int track, Field field) const
{
    VERIFY_INDEX(track);
    return d.all.value(dataIndex(track)).value(field);
}

QVariant Playlist::field(const QString &file, Field field) const
{
    return fields(file, field).value(field);
}

bool Playlist::remove(int track, int size)
{
    VERIFY_INDEX(track);
    VERIFY_INDEX(track + size - 1);
    if (!d.mapping.isEmpty()) {
        qDebug("%s %d: if (!d.mapping.isEmpty()) {", __FILE__, __LINE__);
        return false;
    }
    d.all.erase(d.all.begin() + track, (d.all.begin() + track + size));
    return true;
}

bool Playlist::move(int from, int to)
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(to);
    if (!d.mapping.isEmpty()) {
        qDebug("%s %d: if (!d.mapping.isEmpty()) {", __FILE__, __LINE__);
        return false;
    }
    d.all.move(from, to);
    emit trackChanged(from);
    emit trackChanged(to);
    return true;
}

bool Playlist::swap(int from, int to)
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(to);
    d.all.swap(from, to);
    emit trackChanged(from);
    emit trackChanged(to);
    return true;
}

void Playlist::setFilter(const QRegExp &rx, uint fields)
{
    Q_ASSERT(fields != None || rx.isEmpty());
    d.filter = rx;
    d.filterFields = fields;
    // updateView
}

QRegExp Playlist::filter() const
{
    return d.filter;
}

QList<QHash<Playlist::Field, QVariant> > Playlist::fields(int from, int size, uint types) const
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(from + size - 1);
    QList<QHash<Playlist::Field, QVariant> > ret;
    for (int i=from; i<size + from; ++i) {
        ret += fields(i, types);
    }
    return ret;
}

void Playlist::sort(Field f, Qt::SortOrder sortorder)
{
}

uint Playlist::filterFields() const
{
    return d.filterFields;
}

bool Playlist::filter(int index) const
{
    VERIFY_INDEX(index);
    return filter(fields(index, d.filterFields));
}

bool Playlist::filter(const QString &file) const
{
    return filter(fields(file, d.filterFields));
}

bool Playlist::filter(const QHash<Field, QVariant> &fields) const
{
    if (d.filter.isEmpty())
        return true;
    for (QHash<Field, QVariant>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
        if (it.key() & d.filterFields && it.value().toString().contains(d.filter)) {
            return true;
        }
    }
    return false;
}
