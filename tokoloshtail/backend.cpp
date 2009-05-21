#include "backend.h"
#include <config.h>

Backend *Backend::inst = 0;
Backend *Backend::instance()
{
    return inst;
}

Backend::Backend()
{
    Q_ASSERT(!inst);
    inst = this;
}

int Backend::count() const
{
    return playlistData.tracks.size();
}

QString Backend::currentTrackName() const
{
    return playlistData.tracks.value(playlistData.current);
}

int Backend::currentTrackIndex() const
{
    return playlistData.current;
}

QStringList Backend::tracks(int start, int count) const
{
    return playlistData.tracks.mid(start, count);
}

void Backend::setCurrentTrack(int index)
{
    if (index >= 0 && index < playlistData.tracks.size()) {
        playlistData.current = index;
//        emit currentTrackChanged(index, playlistData.tracks.at(index), trackData(playlistData.tracks.at(index)));
    } else {
        qDebug("%s %d: } else {", __FILE__, __LINE__);
    }
}

void Backend::setCurrentTrack(const QString &name)
{
    const int idx = playlistData.tracks.indexOf(name);
    if (idx != -1) {
        setCurrentTrack(idx);
    } else {
        qDebug("%s %d: } else {", __FILE__, __LINE__);
    }
}

int Backend::indexOfTrack(const QString &name) const
{
    return playlistData.tracks.indexOf(name);
}

void Backend::requestTrackData(const QString &filepath, uint trackInfo)
{
    TrackData data;
    if (trackInfo & FilePath)
        data.path = filepath;
    if (trackInfo & PlaylistIndex)
        data.playlistIndex = indexOfTrack(filepath);
    enum { BackendTypes = Title|TrackLength|Artist|Year|Genre|TrackNumber };
    if (trackInfo & BackendTypes) {
        trackData(&data, filepath, trackInfo); // ### check return value?
    }
    emit trackData(filepath, qVariantFromValue(data));
}

void Backend::requestTracknames(int from, int size)
{
    if (!playlistData.tracks.isEmpty()) {
        from = qBound(0, from, playlistData.tracks.size() - 1);
        if (size < 0) {
            size = playlistData.tracks.size() - from;
        } else {
            size = qMin(playlistData.tracks.size() - from, size);
        }
        emit trackNames(from, playlistData.tracks.mid(from, size));
    }
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

void Backend::load(const QString &path)
{
    static const bool resolveSymlinks = Config::isEnabled("resolvesymlinks");
    const QFileInfo fi = resolveSymlinks ? ::resolveSymlink(path) : QFileInfo(path);
    Q_ASSERT(!fi.isSymLink());
    if (fi.exists()) {
        if (fi.isFile()) {
//            data.playlistData.tracks.append(path);
        } else {

        }
    }
}

void Backend::removeTrack(int index)
{

}
void Backend::swap(int from, int to)
{

}
