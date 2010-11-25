Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "xinebackend.h"
#include <xine.h>
#include <xine/xineutils.h>
#include <tail.h>

#ifndef XINE_STREAM_COUNT
#define XINE_STREAM_COUNT 3
#endif

struct Node {
    Node() : stream(0), next(0) {}

    xine_stream_t *stream;
    QUrl url;
    Node *next;
};

void swap(Node *left, Node *right)
{
    qSwap(left->stream, right->stream);
    qSwap(left->url, right->url);
    qSwap(left->next, right->next);
}

static bool initStream(Node *node, const QUrl &url)
{
    Q_ASSERT(node->stream);
    xine_close(node->stream);

    if (!xine_open(node->stream, url.toString().toLocal8Bit().constData())) {
//        fprintf(stderr, "Unable to open path '%s'\n", fileName.toLocal8Bit().constData());
        return false;
    }
    Q_ASSERT(node->url != url);
    node->url = url;
    return true;
}

struct Private : public QObject
{
    Private() : xine(0), first(0), ao_port(0), event_queue(0),
                status(Backend::Uninitalized), error(XINE_ERROR_NONE),
                progressType(Backend::Seconds), pendingProgress(0)
    {}

    xine_stream_t *stream(const QUrl &url, Node **out = 0)
    {
        if (main.url == url) {
            if (out)
                *out = &main;
            return main.stream;
        } else if (!first) {
            if (!::initStream(&main, url)) {
                if (out)
                    *out = 0;
                return 0;
            }
            if (out)
                *out = &main;
            return main.stream;
        }
        Node *last = 0;
        Node *node = first;
        Q_ASSERT(first);
        forever {
            if (node->url == url) {
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
        if (!::initStream(node, url)) {
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
        if (!stream)
            return;
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

XineBackend::XineBackend(QObject *tail)
    : Backend("XineBackend", tail), d(new Private)
{
}

XineBackend::~XineBackend()
{
    delete d;
}

bool XineBackend::initBackend()
{
    if (d->status != Uninitalized) {
        return true;
    }
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

    if (!(d->event_queue = xine_event_new_queue(d->main.stream))) {
        d->error = -1;
        return false;
    }

    d->first = 0;
    Node **node = &d->first;

    for (int i=0; i<XINE_STREAM_COUNT; ++i) {
        *node = new Node;
        (*node)->stream = xine_stream_new(d->xine, d->ao_port, NULL);
        // can I pass 0 as ao_port?
        if (!(*node)->stream) {
            d->updateError(0);
            return false;
        }
        node = &((*node)->next);
    }

    d->status = Stopped;

    return true;
}

void XineBackend::shutdown()
{
    if (d->status == Uninitalized)
        return;
    if (d->event_queue) {
        xine_event_dispose_queue(d->event_queue);
        d->event_queue = 0;
    }


    if (d->main.stream) {
        xine_stop(d->main.stream);
        xine_close(d->main.stream);
        if (d->ao_port) {
            xine_close_audio_driver(d->xine, d->ao_port);
            d->ao_port = 0;
        }
//        xine_dispose(d->main.stream); // ### this hangs
        d->main.stream = 0;
    }

    while (d->first) {
        if (d->first->stream) {
            xine_close(d->first->stream);
            xine_dispose(d->first->stream);
        }
        Node *tmp = d->first;
        d->first = d->first->next;
        delete tmp;
    }

    xine_exit(d->xine);
    d->xine = 0;
    d->status = Uninitalized;
    statusChanged(d->status);
}

typedef QVariant (*Xine_Get_Meta_Info_Func)(xine_stream_t *stream, int info);
typedef void (*VariableSetter_Func)(void *target, const QVariant &source);
static QVariant xine_get_meta_data(xine_stream_t *stream, int info)
{
    const char *utf8 = xine_get_meta_info(stream, info);
    return QString::fromUtf8(utf8);
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

template <typename T> class Setter
{
public:
    static void set(void *target, const QVariant &source)
    {
        Q_ASSERT(target);
        if (source.canConvert<T>()) {
            T **tt = reinterpret_cast<T**>(&target);
            *(*tt) = qVariantValue<T>(source);
        }
    }
};

bool XineBackend::trackData(TrackData *data, const QUrl &url, int mask) const
{
    if (status() == Uninitalized)
        return false;
    enum { BackendTypes = Title|TrackLength|Artist|Year|Genre|AlbumIndex };
    if (!(mask & BackendTypes)) // shouldn't really happen
        return true;

    xine_stream_t *stream = d->stream(url);
    if (!stream)
        return false; // set error codes? warn?

    struct {
        const TrackInfo field;
        const int id;
        const Xine_Get_Meta_Info_Func info;
        const VariableSetter_Func setter;
        void *target;
    } static const fields[] = {
        { Title, XINE_META_INFO_TITLE, xine_get_meta_data, Setter<QString>::set, &data->title },
        { TrackLength, 2, xine_get_track_length, Setter<int>::set, &data->trackLength },
        { Artist, XINE_META_INFO_ARTIST, xine_get_meta_data, Setter<QString>::set, &data->artist },
        { Year, XINE_META_INFO_YEAR, xine_get_meta_data, Setter<int>::set, &data->year },
        { Genre, XINE_META_INFO_GENRE, xine_get_meta_data, Setter<QString>::set, &data->genre },
        { AlbumIndex, XINE_META_INFO_TRACK_NUMBER, xine_get_meta_data, Setter<int>::set, &data->albumIndex },
        { None, -1, 0, 0, 0 }
    };

    for (int i=0; fields[i].info; ++i) {
        if (mask & fields[i].field) {
            const QVariant v = fields[i].info(stream, fields[i].id);
            fields[i].setter(fields[i].target, v);
        }
    }

    return true;
}

bool XineBackend::isValid(const QUrl &url) const
{
    return status() != Uninitalized && (url.toLocalFile().isEmpty() || d->stream(url)); // ### should maybe not do this for remote files
}


void XineBackend::play()
{
    if (status() != Uninitalized) {
        if (status() == Paused) {
            qWarning("%s:%d unimplemented. Needs to restart at the right time", __FILE__, __LINE__);
        }
        const bool ok = xine_play(d->main.stream, 0, 0);
        if (ok) {
            d->pollTimer.start(500, d); // what is this timer doing?
            d->status = Playing;
            statusChanged(d->status);
        } else {
            d->updateError(d->main.stream);
        }
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
        d->status = Paused;
        statusChanged(d->status);
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
        d->status = Stopped;
        statusChanged(d->status);
    }

}

bool XineBackend::loadUrl(const QUrl &url)
{
    stop();
    Node *node;
    xine_stream_t *s = d->stream(url, &node);
    if (!s)
        return false;
    if (node != &d->main) {
        ::swap(node, &d->main);
    }

    return true;
}

int XineBackend::status() const
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

void XineBackend::setMute(bool on)
{
    xine_set_param(d->main.stream, XINE_PARAM_AUDIO_MUTE, on ? 1 : 0);
    d->updateError(d->main.stream);
}

bool XineBackend::isMute() const
{
    const int ret = xine_get_param(d->main.stream, XINE_PARAM_AUDIO_MUTE);
    d->updateError(d->main.stream);
    return ret == 1;
}

void XineBackend::setProgress(int type, int progress)
{
    if (status() != Playing) {
        d->progressType = static_cast<ProgressType>(type);
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

int XineBackend::progress(int type)
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
    case -1: return "Unknown error";
    case XINE_ERROR_NO_INPUT_PLUGIN: return "XINE_ERROR_NO_INPUT_PLUGIN";
    case XINE_ERROR_NO_DEMUX_PLUGIN: return "XINE_ERROR_NO_DEMUX_PLUGIN";
    case XINE_ERROR_DEMUX_FAILED: return "XINE_ERROR_DEMUX_FAILED";
    case XINE_ERROR_MALFORMED_MRL: return "XINE_ERROR_MALFORMED_MRL";
    case XINE_ERROR_INPUT_FAILED: return "XINE_ERROR_INPUT_FAILED";
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

int XineBackend::flags() const
{
    return SupportsEqualizer;
}

QHash<int, int> XineBackend::equalizerSettings() const
{
    QHash<int, int> ret;
    static const int hz[] = { 30, 60, 126, 250, 500, 1000, 2000, 4000, 8000, 16000, -1 };
    for (int i=0; hz[i] != -1; ++i) {
        ret[hz[i]] = xine_get_param(d->main.stream, XINE_PARAM_EQ_30HZ + i);
    }
    d->updateError(d->main.stream);
    return ret;
}

void XineBackend::setEqualizerSettings(const QHash<int, int> &eq)
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
    BackendPlugin *createTokoloshBackendInterface()
    {
        return new XineBackendPlugin;
    }
};


