Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <global.h>

class Tail;
class Backend
{
public:
    virtual ~Backend()
    {
    }
    enum Status {
        Uninitalized,
        Playing,
        Paused,
        Stopped,
        Undefined
    };

    enum ProgressType {
        Seconds,
        Portion // ### 100th of a percent. Xine uses 0-65535 should we too?. Not a good name btw
    };

    enum Capability {
        NoCapabilities = 0x000,
        SupportsEqualizer = 0x001,
        SupportsRemoteFiles = 0x002
    };

    enum Event {
        NoEvent = 0,
        SongFinished,
        TrackChanged,
        SongPaused,
        PlayResumed,
        SongStopped,
        VolumeChanged,
        MuteChanged,
        EqualizerChanged,
        ProgressChanged // ### ????
    };

    virtual bool trackData(TrackData *data, const QUrl &path, int types = All) const = 0;
    virtual void shutdown() = 0;
    virtual int capabilities() const { return None; }
    virtual bool isValid(const QUrl &url) const = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void setProgress(int type, int progress) = 0;
    virtual int progress(int type) = 0;
    virtual void stop() = 0;
    virtual bool loadUrl(const QUrl &fileName) = 0;
    virtual int status() const = 0;
    virtual int volume() const = 0;
    virtual void setVolume(int vol) = 0;
    virtual bool initBackend() = 0;
    virtual QString errorMessage() const { return QString(); }
    virtual int errorCode() const { return 0; }
    virtual void setMute(bool on) = 0;
    virtual bool isMute() const = 0;
    virtual QHash<int, int> equalizerSettings() const { return QHash<int, int>(); }
    virtual void setEqualizerSettings(const QHash<int, int> &) {}
    QString name() const { return d.name; }
protected:
    void statusChanged(int status)
    {
        Q_ASSERT(d.tail);
        QMetaObject::invokeMethod(d.tail, "statusChanged", Q_ARG(int, status));
    }
    Backend(const QString &name, QObject *tail)
    {
        Q_ASSERT(tail);
        d.name = name;
        d.tail = tail;
    }


    struct Data {
        QString name;
        QObject *tail;
    } d;
};

#endif
