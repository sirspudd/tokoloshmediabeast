#include "backend.h"

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

QByteArray Backend::trackData(const QString &fileName) const
{
//    return
    // call through to field.* here
}

QStringList Backend::tracks(int start, int count) const
{
    return playlistData.tracks.mid(start, count);
}

void Backend::setCurrentTrack(int index)
{
    if (index >= 0 && index < playlistData.tracks.size()) {
        playlistData.current = index;
        emit currentTrackChanged(index, playlistData.tracks.at(index), trackData(playlistData.tracks.at(index)));
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
