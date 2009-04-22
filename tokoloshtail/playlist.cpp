#include "playlist.h"
#include <xine.h>

#define VERIFY_INDEX(idx) { Q_ASSERT_X(idx >= 0 && idx < count(), \
                                       qPrintable(QString("Index out of range %1 (0-%2)").arg(idx).arg(count())), \
                                       __FUNCTION__); }

Playlist::Playlist(QObject *parent)
    : QObject(parent)
{
    d.filterFields = All;
    d.sortField = None;
    d.sortOrder = Qt::AscendingOrder;
}

bool Playlist::validTrack(const QString &file) const
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
    const QHash<Field, QVariant> f = fields(path);
    if (f.isEmpty())
        return;

    const int c = count();
    d.all.append(f);
    if (!d.mapping.isEmpty() && filter(f)) {
        d.mapping.append(c);
    }
    if (count() != c) {
        Q_ASSERT(count() - 1 == c);
        emit countChanged(c + 1);
    }
}

QHash<Playlist::Field, QVariant> Playlist::fields(const QString &path, uint fields) const
{
    QHash<Field, QVariant> ret;
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
    const bool hadFilter = d.filter.isEmpty();
    Q_ASSERT(!rx.isEmpty() || fields == None); // no combination of valid regexp and fields == None
    d.filter = rx;
    d.filterFields = fields;
    if (!hadFilter) {
//        if (d.filter.isEmpty

    }
//    return;

//     if (!hadFilter) {

//     int index = 0;
//     for (int i=0; i<d.all.size(); ++i) {
//     }



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

template <typename T>
class Sorter
{
public:
    Sorter(Playlist::Field f, Qt::SortOrder o) : field(f), order(o) {}
    inline bool operator()(const QHash<Playlist::Field, QVariant> &l, const QHash<Playlist::Field, QVariant> &r) const
    {
        return (qVariantValue<T>(l.value(field)) < qVariantValue<T>(r.value(field))) == (order == Qt::AscendingOrder);
    }
private:
    const Playlist::Field field;
    const Qt::SortOrder order;
};

static inline QVariant::Type type(Playlist::Field field)
{
    switch (field) {
    case Playlist::All:
    case Playlist::None:
        Q_ASSERT(0);
        break;
    case Playlist::FileName:
    case Playlist::FilePath:
    case Playlist::FileDirectory:
    case Playlist::TrackName:
    case Playlist::Artist:
    case Playlist::Album:
    case Playlist::Genre:
        return QVariant::String;
    case Playlist::TrackLength:
    case Playlist::Year:
    case Playlist::TrackNumber:
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

void Playlist::sort(Field field, Qt::SortOrder sortOrder)
{
    if (d.sortField == field) {
        if (d.sortOrder == sortOrder)
            return;
        d.sortOrder = sortOrder;
        if (!d.all.isEmpty()) {
            reverse(d.all);

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
                qSort(d.all.begin(), d.all.end(), Sorter<QString>(field, sortOrder));
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
