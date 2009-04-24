#include "playlist.h"
#include <xine.h>
#include "backend.h"

#define VERIFY_INDEX(idx) { Q_ASSERT_X(idx >= 0 && idx < count(), \
                                       qPrintable(QString("Index out of range %1 (0-%2)").arg(idx).arg(count())), \
                                       __FUNCTION__); }

Playlist::Playlist(QObject *parent)
    : QObject(parent)
{
    d.backend = Backend::instance();
    d.filterFields = All;
    d.sortField = None;
    d.sortOrder = Qt::AscendingOrder;
}

bool Playlist::validTrack(const QString &file) const
{
    // should this be a separate function in Backend or should I just
    // ask for Title and say it's valid if it has it?

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


QStringList Playlist::validTracks(const QString &path, bool recurse) const
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
            ret += validTracks(abs, recurse);
        } else if (validTrack(abs)) {
            ret.append(abs);
        }
    }
    return ret;
}

int Playlist::count() const
{
    return d.filter.isEmpty() ? d.all.size() : d.mapping.size();
}

void Playlist::addTrack(const QString &path)
{
    const QHash<TrackInfo, QVariant> f = fields(path);
    if (f.isEmpty())
        return;

    const int c = count();
    d.all.append(f);
    if (!d.filter.isEmpty() && filter(f)) {
        d.mapping.append(c);
    }
    if (count() != c) {
        Q_ASSERT(count() - 1 == c);
        emit countChanged(c + 1);
        if (d.sortField != None) {
            // we're always appending so this breaks the sort. Could
            // insertion-sort it though.
            d.sortField = None;
            emit sortChanged(d.sortField, d.sortOrder);
        }
    }
}

QHash<TrackInfo, QVariant> Playlist::fields(const QString &path, uint fields) const
{
    if (path == d.cache.cachedTrack) {
        if (fields == All)
            return d.cache.cachedFields;
        QHash<TrackInfo, QVariant> ret;
        if (fields != All) {
            for (QHash<TrackInfo, QVariant>::const_iterator it = d.cache.cachedFields.begin(); it != d.cache.cachedFields.end(); ++it) {
                if (fields & it.key())
                    ret[it.key()] = it.value();
            }
        }
        return ret;
    }
    QHash<TrackInfo, QVariant> ret;
    if (!validTrack(path))
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
    if (fields == All) {
        d.cache.cachedFields = ret;
        d.cache.cachedTrack = path;
    }

    return ret;
}

QHash<TrackInfo, QVariant> Playlist::fields(int track, uint fields) const
{
    VERIFY_INDEX(track);
    const int idx = dataIndex(track);
    const QHash<TrackInfo, QVariant> &orig = d.all.at(idx);
    if (fields == All)
        return orig;
    QHash<TrackInfo, QVariant> f;
    for (QHash<TrackInfo, QVariant>::const_iterator it = orig.begin(); it != orig.end(); ++it) {
        if (fields & it.key())
            f[it.key()] = it.value();
    }
    return f;
}

QVariant Playlist::field(int track, TrackInfo field) const
{
    VERIFY_INDEX(track);
    return d.all.value(dataIndex(track)).value(field);
}

QVariant Playlist::field(const QString &file, TrackInfo field) const
{
    return fields(file, field).value(field);
}

bool Playlist::remove(int track, int size)
{
    VERIFY_INDEX(track);
    VERIFY_INDEX(track + size - 1);
    const int c = count();
    d.all.erase(d.all.begin() + track, (d.all.begin() + track + size));
    int removeFrom = -1;
    int removeLast = -1;
    const int s = d.mapping.size();
    for (int i=0; i<s; ++i) {
        int &val = d.mapping[i++];
        if (val >= track + size) {
            val -= size;
            if (removeLast == -1)
                removeLast = i - 1;
        } else if (val >= track) {
            if (removeFrom == -1)
                removeFrom = i;
        }
    }
    if (removeFrom != -1) {
        // emit ///
        d.mapping.erase(d.mapping.begin() + removeFrom, d.mapping.begin() + removeLast);
    }
    if (c != count()) {
        emit countChanged(count());
    }

    return true;
}

bool Playlist::move(int from, int to)
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(to);
    if (!d.mapping.isEmpty()) {
        // ### need to fix
        qDebug("%s %d: if (!d.mapping.isEmpty()) {", __FILE__, __LINE__);
        return false;
    } else {
        d.all.move(from, to);
    }


    const int c = count();
    const int start = qMin(from, to);
    emit tracksChanged(start, c - start);
    if (d.sortField != None) {
        d.sortField = None;
        emit sortChanged(d.sortField, d.sortOrder);
    }

    return true;
}

bool Playlist::swap(int from, int to)
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(to);
    if (!d.mapping.isEmpty()) {
        // ### need to fix
        qDebug("%s %d: if (!d.mapping.isEmpty()) {", __FILE__, __LINE__);
        return false;
    } else {
        d.all.swap(from, to);
    }
    emit trackChanged(from);
    emit trackChanged(to);
    if (d.sortField != None) {
        d.sortField = None;
        emit sortChanged(d.sortField, d.sortOrder);
    }
    return true;
}

void Playlist::setFilter(const QRegExp &rx, uint fields)
{
    const bool hadFilter = d.filter.isEmpty();
    Q_ASSERT(!rx.isEmpty() || fields == None);
    d.filter = rx;
    d.filterFields = fields;
    const int c = count();
    const int s = d.all.size();
    if (!hadFilter) {
        Q_ASSERT(d.mapping.isEmpty());
        if (d.filter.isEmpty())
            return; // still no filter
        for (int i=0; i<s; ++i) {
            if (filter(d.all.at(i)))
                d.mapping.append(i);
        }
    } else if (d.filter.isEmpty()) {
        d.mapping.clear();
    } else {
        int idx = 0;
        for (int i=0; i<s; ++i) {
            const bool old = (d.mapping.value(idx, -1) == i);
            if (filter(d.all.at(i)) != old) {
                if (old) {
                    d.mapping.removeAt(idx);
                } else {
                    d.mapping.insert(idx++, i);
                }
            } else {
                ++idx;
            }
        }
    }
    if (c != count()) {
        emit filterChanged(d.filter, d.filterFields);
        emit countChanged(c);
    }
}

QRegExp Playlist::filter() const
{
    return d.filter;
}

QList<QHash<TrackInfo, QVariant> > Playlist::fields(int from, int size, uint types) const
{
    VERIFY_INDEX(from);
    VERIFY_INDEX(from + size - 1);
    QList<QHash<TrackInfo, QVariant> > ret;
    for (int i=from; i<size + from; ++i) {
        ret += fields(i, types);
    }
    return ret;
}

template <typename T>
class Sorter
{
public:
    Sorter(TrackInfo f, Qt::SortOrder o) : field(f), order(o) {}
    inline bool operator()(const QHash<TrackInfo, QVariant> &l, const QHash<TrackInfo, QVariant> &r) const
    {
        return (qVariantValue<T>(l.value(field)) < qVariantValue<T>(r.value(field))) == (order == Qt::AscendingOrder);
    }
private:
    const TrackInfo field;
    const Qt::SortOrder order;
};

static inline QVariant::Type type(TrackInfo field)
{
    switch (field) {
    case All:
    case None:
        Q_ASSERT(0);
        break;
    case FileName:
    case FilePath:
    case FileDirectory:
    case TrackName:
    case Artist:
    case Album:
    case Genre:
        return QVariant::String;
    case TrackLength:
    case Year:
    case TrackNumber:
        return QVariant::Int;
    }
    return QVariant::Invalid;
}

template <typename T> inline void reverse(QList<T> &list)
{
    const int size = list.size();
    for (int i=0; i<size / 2; ++i) {
        list.swap(i, size - i - 1);
    }
}

void Playlist::sort(TrackInfo field, Qt::SortOrder sortOrder)
{
    if (d.sortField == field) {
        if (d.sortOrder == sortOrder)
            return;
        d.sortOrder = sortOrder;
        if (!d.all.isEmpty()) {
            ::reverse(d.all);
            if (!d.mapping.isEmpty()) {
                // is this stuff right?
                const int size = d.all.size();
                const int mappingSize = d.mapping.size() / 2;
                for (int i=0; i<mappingSize; ++i) {
                    const int tmp = size - d.mapping.at(i);
                    d.mapping[i] = size - d.mapping.at(mappingSize - i - 1);
                    d.mapping[mappingSize - i - 1] = tmp;
                }
            }
        }
    } else {
        const QVariant::Type type = ::type(field);
        Q_ASSERT(type != QVariant::Invalid);
        d.sortField = field;
        d.sortOrder = sortOrder;
        if (!d.all.isEmpty()) {
            switch (type) {
            case QVariant::String:
                qSort(d.all.begin(), d.all.end(), Sorter<QString>(field, sortOrder));
                break;
            case QVariant::Int:
                qSort(d.all.begin(), d.all.end(), Sorter<int>(field, sortOrder));
                break;
            default:
                Q_ASSERT(0);
                break;
            }
            if (!d.mapping.isEmpty()) {
                int index = 0;
                const int size = d.all.size();
                for (int i=0; i<size; ++i) {
                    if (filter(d.all.at(i))) {
                        d.mapping[index++] = i;
                    }
                }
            }
        }
    }
    emit sortChanged(d.sortField, d.sortOrder);
}

uint Playlist::filterFields() const
{
    return d.filterFields;
}

bool Playlist::filter(const QHash<TrackInfo, QVariant> &fields) const
{
    if (d.filter.isEmpty())
        return true;
    for (QHash<TrackInfo, QVariant>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
        if (it.key() & d.filterFields && it.value().toString().contains(d.filter)) {
            return true;
        }
    }
    return false;
}
