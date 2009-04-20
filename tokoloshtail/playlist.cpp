#include "playlist.h"

#define VERIFY_INDEX(idx) { Q_ASSERT_X(idx >= 0 && idx < d.current.size(), \
                                       qPrintable(QString("Index out of range %1 (0-%2)").arg(idx).arg(d.current.size())), \
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
    return d.current.size();
}

int Playlist::insertSong(int index, const QString &path)
{
#warning This stuff must insert into both all and current (if it matches the current filter)
    if (index < 0)
        index = d.current.size(); // what if one
    Q_ASSERT(index <= count());
    QHash<Field, QVariant> f = fields(path);
    if (f.isEmpty())
        return -1;
    d.current.insert(index, f);
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
    if (fields & FileName)
        ret[FileName] = fi.fileName();
    if (fields & FilePath)
        ret[FilePath] = fi.path();
    // need to ask xine for stuff
    return ret;
}

QHash<Playlist::Field, QVariant> Playlist::fields(int track, uint fields) const
{
    VERIFY_INDEX(track);
    QHash<Field, QVariant> f = d.current.value(track);
    if (fields != All) {
        foreach(Field key, f.keys()) { // not the most efficient way maybe
            if (!(fields & key))
                f.remove(key);
        }
    }
    return f;
}

QVariant Playlist::field(int track, Field field) const
{
    VERIFY_INDEX(track);
    return d.current.value(track).value(field);
}

QVariant Playlist::field(const QString &file, Field field) const
{
    return fields(file, field).value(field);
}

bool Playlist::remove(int track, int count)
{
    VERIFY_INDEX(track);
    VERIFY_INDEX(track + count - 1);
}

bool Playlist::move(int from, int to)
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(to);
}

bool Playlist::swap(int from, int to)
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(to);
}

void Playlist::setFilter(const QRegExp &rx, uint fields)
{

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
