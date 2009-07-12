#include "phononbackend.h"

struct Private
{
    Private()
        : audioOutput(0)
    {}

    Phonon::MediaObject main, aux;
    Phonon::AudioOutput *audioOutput;
};

PhononBackend::PhononBackend(QObject *parent)
    : Backend(parent), d(new Private)
{
}

PhononBackend::~PhononBackend()
{
    delete d;
}

bool PhononBackend::initBackend()
{
}

void PhononBackend::shutdown()
{
}

bool PhononBackend::trackData(TrackData *data, const QUrl &url, int mask) const
{
    if (status() == Uninitalized)
        return false;
    enum { BackendTypes = Title|TrackLength|Artist|Year|Genre|AlbumIndex };
    if (!(mask & BackendTypes)) // shouldn't really happen
        return true;

    struct {
        const TrackInfo field;
        const Phonon::MetaData metaData;
    } static const fields[] = {
        { Title, Phonon::TitleMetaData },
        { Artist, Phonon::ArtistMetaData },
        { Album, Phonon::AlbumMetaData },
        { Year, Phonon::DateMetaData },
        { Genre, Phonon::GenreMetaData },
        { AlbumIndex, Phonon::TracknumberMetaData },
        { None, (Phonon::MetaData)-1 }
    };

    Phonon::MediaObject *const objects[] = { &d->main, &d->aux, 0 };
    Phonon::MediaObject *object = 0;

    for (int i=0; objects[i]; ++i) {
        const Phonon::MediaSource source = objects[i]->currentSource();
        if (source.url() == url) {
            object = objects[i];
            break;
        }
    }
    if (!object) {
        d->aux.setCurrentSource(Phonon::MediaSource(url));
        if (!d->aux.isValid()) {
            return false;
        }
        object = &d->aux;
    }

    for (int i=0; fields[i].field != None; ++i) {
        if (mask & fields[i].field) {
            const QString v = object->metaData(fields[i].metaData).value(0);
            if (!v.isNull())
                data->setData(fields[i].field, v);
        }
    }

    return true;
}

bool PhononBackend::isValid(const QUrl &fileName) const
{
//    return status() != Uninitalized && d->main.isValid();
}


void PhononBackend::play()
{
//     if (status() != Uninitalized) {
//         const bool ok = xine_play(d->main.stream, 0, 0);
//         if (ok) {
//             d->pollTimer.start(500, this);
//             d->status = Playing;
//             emit statusChanged(d->status);
//         } else {
//             d->updateError(d->main.stream);
//         }
//     }
}

void PhononBackend::pause()
{
//     if (status() == Playing) {
//         d->pollTimer.stop();
//         d->pendingProgress = progress(Portion);
//         d->progressType = Portion;
//         d->pollTimer.stop();
//         xine_stop(d->main.stream);
//         d->updateError(d->main.stream);
//         d->status = Idle;
//         emit statusChanged(d->status);
//     }
}

void PhononBackend::stop()
{
//     if (status() == Playing) {
//         d->pendingProgress = 0;
//         d->progressType = Seconds;
//         xine_stop(d->main.stream);
//         d->updateError(d->main.stream);
//         d->pollTimer.stop();
//         d->status = Idle;
//         emit statusChanged(d->status);
//     }
}

bool PhononBackend::loadUrl(const QUrl &url)
{
//     stop();
//     Node *node;
//     xine_stream_t *s = d->stream(fileName, &node);
//     if (!s)
//         return false;
//     if (node != &d->main) {
//         ::swap(node, &d->main);
// //        emit currentTrackChanged(fileName);
//     }

//    return true;
    return false;
}

int PhononBackend::status() const
{
    switch (d->main.state()) {
    case Phonon::PausedState:
        return Paused;
    case Phonon::StoppedState:
    case Phonon::LoadingState:
    case Phonon::ErrorState:
        return Stopped;
    case Phonon::PlayingState:
    case Phonon::BufferingState:
        return Playing;
    }
}

int PhononBackend::volume() const
{
//     return d->audioOutput ? d->audioOutput->volume()2
//     return ret;
}

void PhononBackend::setVolume(int vol)
{
//     xine_set_param(d->main.stream, XINE_PARAM_AUDIO_VOLUME, vol);
//     d->updateError(d->main.stream);
}

void PhononBackend::setMute(bool on)
{
//     xine_set_param(d->main.stream, XINE_PARAM_AUDIO_MUTE, on ? 1 : 0);
//     d->updateError(d->main.stream);
}

bool PhononBackend::isMute() const
{
//     const int ret = xine_get_param(d->main.stream, XINE_PARAM_AUDIO_MUTE);
//     d->updateError(d->main.stream);
//     return ret == 1;
}

void PhononBackend::setProgress(int type, int progress)
{
//     if (status() != Playing) {
//         d->progressType = static_cast<ProgressType>(type);
//         d->pendingProgress = progress;
//     } else {
//         d->progressType = Seconds;
//         d->pendingProgress = 0;
//         int start_pos = 0;
//         int start_time = 0;
//         if (type == Seconds) {
//             start_time = progress * 1000;
//         } else {
//             start_pos = int(double(progress) / 10000.0 * 65535.0);
//         }
//         xine_play(d->main.stream, start_pos, start_time);
//         d->updateError(d->main.stream);
//     }
}

int PhononBackend::progress(int type)
{
//     const QVariant var = ::xine_get_track_length(d->main.stream, type == Seconds ? 1 : 0);
//     if (var.isNull())
//         return -1;
//     if (type == Seconds) {
//         return var.toInt() * 1000;
//     } else {
//         return int(double(var.toInt()) * 10000.0 / 65535.0);
//         // 100th of a percent
//     }
}
QString PhononBackend::errorMessage() const
{
//     switch (errorCode()) {
//     case XINE_ERROR_NONE: return QString();
//     case -1: return QLatin1String("Unknown error");
//     case XINE_ERROR_NO_INPUT_PLUGIN: return QLatin1String("XINE_ERROR_NO_INPUT_PLUGIN");
//     case XINE_ERROR_NO_DEMUX_PLUGIN: return QLatin1String("XINE_ERROR_NO_DEMUX_PLUGIN");
//     case XINE_ERROR_DEMUX_FAILED: return QLatin1String("XINE_ERROR_DEMUX_FAILED");
//     case XINE_ERROR_MALFORMED_MRL: return QLatin1String("XINE_ERROR_MALFORMED_MRL");
//     case XINE_ERROR_INPUT_FAILED: return QLatin1String("XINE_ERROR_INPUT_FAILED");
//     default:
//         break;
//     }
//     Q_ASSERT(0);
//     return QString();
}
int PhononBackend::errorCode() const
{
//    return d->error;
}

int PhononBackend::flags() const
{
    return NoCapabilities;
}

#if 0
QHash<int, int> PhononBackend::equalizerSettings() const
{
    QHash<int, int> ret;
    static const int hz[] = { 30, 60, 126, 250, 500, 1000, 2000, 4000, 8000, 16000, -1 };
    for (int i=0; hz[i] != -1; ++i) {
        ret[hz[i]] = xine_get_param(d->main.stream, XINE_PARAM_EQ_30HZ + i);
    }
    d->updateError(d->main.stream);
    return ret;
}

void PhononBackend::setEqualizerSettings(const QHash<int, int> &eq)
{
    static const int hz[] = { 30, 60, 126, 250, 500, 1000, 2000, 4000, 8000, 16000, -1 };
    for (int i=0; hz[i] != -1; ++i) {
        const int val = eq.value(hz[i], -INT_MAX);
        if (val != -INT_MAX) {
            xine_set_param(d->main.stream, XINE_PARAM_EQ_30HZ + i, val);
        }
    }
    d->updateError(d->main.stream);
}

extern "C" {
    // ### why doesn't this work?
    BackendPlugin *createTokoloshBackendInterface()
    {
        return new PhononBackendPlugin;
    }
};
#endif
