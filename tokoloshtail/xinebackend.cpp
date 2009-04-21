#include "xinebackend.h"
#include <xine.h>
#include <xine/xineutils.h>

#ifndef XINE_STREAM_COUNT
#define XINE_STREAM_COUNT 3
#endif

struct Node {
    Node() : stream(0), next(0) {}

    xine_stream_t *stream;
    QString track;
    Node *next;
};

void swap(Node *left, Node *right)
{
    qSwap(left->stream, right->stream);
    qSwap(left->track, right->track);
    qSwap(left->next, right->next);
}

static bool initStream(Node *node, const QString &fileName)
{
    xine_close(node->stream);
    if (!xine_open(node->stream, qPrintable(fileName))) {
        printf("Unable to open path '%s'\n", qPrintable(fileName));
        return false;
    }
    Q_ASSERT(node->track != fileName);
    node->track = fileName;
    return true;
}

struct Private
{
    Private() : xine(0), first(0), ao_port(0), event_queue(0),
                status(Backend::Uninitalized), error(XINE_ERROR_NONE),
                progressType(Backend::Seconds), pendingProgress(0)
    {}

    xine_stream_t *stream(const QString &fileName, Node **out = 0)
    {
        if (main.track == fileName) {
            if (out)
                *out = &main;
            return main.stream;
        }
        Node *last = 0;
        Node *node = first;
        forever {
            if (node->track == fileName) {
                if (out)
                    *out = node;
                return node->stream;
            } else if (node->next) {
                last = node;
                node = node->next;
            } else {
                break;
            }
        }
        Q_ASSERT(node);
        if (!::initStream(node, fileName)) {
            if (out)
                *out = 0;
            return 0;
        }
        last->next = 0;
        node->next = first;
        first = node;
        if (out)
            *out = node;
        return node->stream;
    }

    inline void updateError(xine_stream_t *stream)
    {
        error = xine_get_error(stream);
    }

    xine_t *xine;
    Node main;
    Node *first;
    xine_audio_port_t *ao_port;
    xine_event_queue_t *event_queue;
    QString filePath, extraPath;
    QBasicTimer pollTimer;
    Backend::Status status;
    int error;
    Backend::ProgressType progressType;
    int pendingProgress;
};

XineBackend::XineBackend()
    : d(new Private)
{
}

bool XineBackend::initBackend()
{
    //Xine initialization
    d->xine = xine_new();

    QByteArray configfile = xine_get_homedir();
    configfile += "/.xine/config";
    xine_config_load(d->xine, configfile.constData());
    xine_init(d->xine);

    if (!(d->ao_port = xine_open_audio_driver(d->xine, "auto" , NULL))) {
        d->error = -1;
        return false;
    }

    if (!(d->main.stream = xine_stream_new(d->xine, d->ao_port, NULL))) {
        d->updateError(0);
        return false;
    }
    Node **node = &d->first;

    for (int i=0; i<XINE_STREAM_COUNT; ++i) {
        *node = new Node;
        (*node)->stream = xine_stream_new(d->xine, NULL /*d->ao_port*/, NULL);
        // can I pass 0 as ao_port?
        if ((*node)->stream) {
            d->updateError(0);
            return false;
        }
        node = &((*node)->next);
    }

    if (!(d->event_queue = xine_event_new_queue(d->main.stream))) {
        d->error = -1;
        return false;
    }

    return true;
}

void XineBackend::shutdown()
{
    if (d->event_queue) {
        xine_event_dispose_queue(d->event_queue);
        d->event_queue = 0;
    }

    if (d->main.stream) {
        xine_close(d->main.stream);
        if (d->ao_port) {
            xine_close_audio_driver(d->xine, d->ao_port);
            d->ao_port = 0;
        }
        xine_dispose(d->main.stream);
        d->main.stream = 0;
    }

    while (d->first) {
        if (d->first->stream) {
            xine_close(d->first->stream);
            xine_dispose(d->first->stream);
        }
        d->first = d->first->next;
    }

    xine_exit(d->xine);
    d->xine = 0;
}

typedef QVariant (*Xine_Get_Meta_Info)(xine_stream_t *stream, int info);

static QVariant xine_get_meta_QString(xine_stream_t *stream, int info)
{
    const char *utf8 = xine_get_meta_info(stream, info);
    return QString::fromUtf8(utf8);
}

static QVariant xine_get_meta_Integer(xine_stream_t *stream, int info)
{
    const char *utf8 = xine_get_meta_info(stream, info);
    return QString::fromUtf8(utf8).toInt();
}


// also used by progress()
static QVariant xine_get_track_length(xine_stream_t *stream, int item)
{
    int ints[3]; // int *pos_stream,  /* 0..65535     */
		 // int *pos_time,    /* milliseconds */
		 // int *length_time) /* milliseconds */

    if (xine_get_pos_length(stream, &ints[0], &ints[1], &ints[2])) {
        Q_ASSERT(item >= 0 && item < 3);
        return ints[item];
    }
    return QVariant();
}

QVariant XineBackend::field(const QString &fileName, Playlist::Field field) const
{
    if (status() == Uninitalized)
        return QVariant();
    xine_stream_t *stream = d->stream(fileName);

    struct {
        const Playlist::Field field;
        const int id;
        const Xine_Get_Meta_Info info;
    } static const data[] = {
        { Playlist::TrackName, XINE_META_INFO_TITLE, xine_get_meta_QString },
        { Playlist::TrackLength, Playlist::TrackLength, xine_get_track_length },
        { Playlist::Artist, XINE_META_INFO_ARTIST, xine_get_meta_QString },
        { Playlist::Album, XINE_META_INFO_ALBUM, xine_get_meta_QString },
        { Playlist::Year, XINE_META_INFO_YEAR, xine_get_meta_Integer },
        { Playlist::Genre, XINE_META_INFO_GENRE, xine_get_meta_QString },
        { Playlist::TrackNumber, XINE_META_INFO_TRACK_NUMBER, xine_get_meta_Integer },
        { Playlist::None, -1, 0 }
    };

    for (int i=0; data[i].info; ++i) {
        if (data[i].field == field)
            return data[i].info(stream, data[i].id);
    }

    return QVariant();
}

void XineBackend::play()
{
    if (status() == Idle) {
        if (xine_play(d->main.stream, 0, 0))
            d->pollTimer.start(500, this);
        d->updateError(d->main.stream);
    }
}

void XineBackend::pause()
{
    if (status() == Playing) {
        d->pollTimer.stop();
        d->pendingProgress = progress(Portion);
        d->progressType = Portion;
        d->pollTimer.stop();
        xine_stop(d->main.stream);
        d->updateError(d->main.stream);
    }
}

void XineBackend::stop()
{
    if (status() == Playing) {
        d->pendingProgress = 0;
        d->progressType = Seconds;
        xine_stop(d->main.stream);
        d->updateError(d->main.stream);
        d->pollTimer.stop();
    }

}

bool XineBackend::load(const QString &fileName)
{
    Node *node;
    xine_stream_t *s = d->stream(fileName, &node);
    if (!s)
        return false;
    if (node != &d->main) {
        ::swap(node, &d->main);
    }
    return true;
}

QString XineBackend::currentTrack() const
{
    return d->main.track;
}

Backend::Status XineBackend::status() const
{
    // could use xine_get_status
    return d->status;
}

int XineBackend::volume() const
{
    const int ret = xine_get_param(d->main.stream, XINE_PARAM_AUDIO_VOLUME);
    d->updateError(d->main.stream);
    return ret;
}

void XineBackend::setVolume(int vol)
{
    xine_set_param(d->main.stream, XINE_PARAM_AUDIO_VOLUME, vol);
    d->updateError(d->main.stream);
}

XineBackend *XineBackend::inst = 0;
XineBackend * XineBackend::instance()
{
    if (!inst) {
        inst = new XineBackend;
        if (!inst->init()) {
            qWarning("Failed to initialize xine backend");
            // delete and set to 0?
        }
    }
    return inst;
}

void XineBackend::setProgress(ProgressType type, int progress)
{
    if (status() != Playing) {
        d->progressType = type;
        d->pendingProgress = progress;
    } else {
        d->progressType = Seconds;
        d->pendingProgress = 0;
        int start_pos = 0;
        int start_time = 0;
        if (type == Seconds) {
            start_time = progress * 1000;
        } else {
            start_pos = int(double(progress) / 10000.0 * 65535.0);
        }
        xine_play(d->main.stream, start_pos, start_time);
        d->updateError(d->main.stream);
    }
}

int XineBackend::progress(ProgressType type)
{
    const QVariant var = ::xine_get_track_length(d->main.stream, type == Seconds ? 1 : 0);
    if (var.isNull())
        return -1;
    if (type == Seconds) {
        return var.toInt() * 1000;
    } else {
        return int(double(var.toInt()) * 10000.0 / 65535.0);
        // 100th of a percent
    }
}
QString XineBackend::errorMessage() const
{
    switch (errorCode()) {
    case XINE_ERROR_NONE: return QString();
    case -1: return QLatin1String("Unknown error");
    case XINE_ERROR_NO_INPUT_PLUGIN: return QLatin1String("XINE_ERROR_NO_INPUT_PLUGIN");
    case XINE_ERROR_NO_DEMUX_PLUGIN: return QLatin1String("XINE_ERROR_NO_DEMUX_PLUGIN");
    case XINE_ERROR_DEMUX_FAILED: return QLatin1String("XINE_ERROR_DEMUX_FAILED");
    case XINE_ERROR_MALFORMED_MRL: return QLatin1String("XINE_ERROR_MALFORMED_MRL");
    case XINE_ERROR_INPUT_FAILED: return QLatin1String("XINE_ERROR_INPUT_FAILED");
    default:
        break;
    }
    Q_ASSERT(0);
    return QString();
}
int XineBackend::errorCode() const
{
    return d->error;
}
