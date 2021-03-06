/*
    Copyright (c) 2010 Anders Bakken
    Copyright (c) 2010 Donald Carr
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer. Redistributions in binary
    form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials
    provided with the distribution. Neither the name of any associated
    organizations nor the names of its contributors may be used to endorse or
    promote products derived from this software without specific prior written
    permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
    NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#ifndef TAIL_H
#define TAIL_H

#include <QtCore>
#include <global.h>
#include "backend.h"

class TagInterface;
struct FunctionNode;
class Tail : public QObject
{
    Q_OBJECT
public:
    Tail(QObject *parent = 0);
    virtual ~Tail();
    bool load(const QUrl &path, bool recursive);
    bool setBackend(Backend *backend);
    void statusChange(int status) { emit statusChanged(status); }
public slots:
    Q_SCRIPTABLE int capabilities() const { Q_ASSERT(d.backend); return d.backend->capabilities(); }
    Q_SCRIPTABLE bool isValid(const QUrl &url) const { Q_ASSERT(d.backend); return d.backend->isValid(url); }
    Q_SCRIPTABLE void play() { Q_ASSERT(d.backend); d.backend->play(); }
    Q_SCRIPTABLE void pause() { Q_ASSERT(d.backend); d.backend->pause(); }
    Q_SCRIPTABLE void setProgress(int type, int progress) { Q_ASSERT(d.backend); d.backend->setProgress(type, progress); }
    Q_SCRIPTABLE int progress(int type) { Q_ASSERT(d.backend); return d.backend->progress(type); }
    Q_SCRIPTABLE void stop() { Q_ASSERT(d.backend); d.backend->stop(); }
    Q_SCRIPTABLE bool loadUrl(const QUrl &url) { Q_ASSERT(d.backend); return d.backend->loadUrl(url); }
    Q_SCRIPTABLE int status() const { Q_ASSERT(d.backend); return d.backend->status(); }
    Q_SCRIPTABLE int volume() const { Q_ASSERT(d.backend); return d.backend->volume(); }
    Q_SCRIPTABLE void setVolume(int vol) { Q_ASSERT(d.backend); d.backend->setVolume(vol); }
    Q_SCRIPTABLE bool initBackend() { Q_ASSERT(d.backend); return d.backend->initBackend(); }
    Q_SCRIPTABLE QString errorMessage() const { Q_ASSERT(d.backend); return d.backend->errorMessage(); }
    Q_SCRIPTABLE int errorCode() const { Q_ASSERT(d.backend); return d.backend->errorCode(); }
    Q_SCRIPTABLE void setMute(bool on) { Q_ASSERT(d.backend); d.backend->setMute(on); }
    Q_SCRIPTABLE bool isMute() const { Q_ASSERT(d.backend); return d.backend->isMute(); }
    Q_SCRIPTABLE QHash<int, int> equalizerSettings() const { Q_ASSERT(d.backend); return d.backend->equalizerSettings(); }
    Q_SCRIPTABLE void setEqualizerSettings(const QHash<int, int> &eq) { Q_ASSERT(d.backend); d.backend->setEqualizerSettings(eq); }

    // playlist stuff
    Q_SCRIPTABLE TrackData trackData(int idx, int fields = All) const;
    Q_SCRIPTABLE TrackData trackData(const QUrl &path, int fields = All) const;
    Q_SCRIPTABLE TrackData trackData(const QString &song, int fields = All) const;
    Q_SCRIPTABLE int count() const;
    Q_SCRIPTABLE QString currentTrackName() const;
    Q_SCRIPTABLE int currentTrackIndex() const;
    Q_SCRIPTABLE QStringList list() const;
    Q_SCRIPTABLE QList<QUrl> tracks(int start, int count) const;
    Q_SCRIPTABLE bool setCurrentTrackIndex(int index);
    Q_SCRIPTABLE bool setCurrentTrack(const QString &name);
    Q_SCRIPTABLE int indexOfTrack(const QUrl &name) const;
    Q_SCRIPTABLE int indexOfTrack(const QString &name) const;
    Q_SCRIPTABLE bool setCWD(const QString &path);
    Q_SCRIPTABLE QString CWD() const;
    Q_SCRIPTABLE QString playlist() const;
    Q_SCRIPTABLE void setPlaylist(const QString &file);
    Q_SCRIPTABLE void clear() { if (count() > 0) removeTracks(0, count()); }
    Q_SCRIPTABLE void quit();
    Q_SCRIPTABLE void sendWakeUp();
    Q_SCRIPTABLE void prev();
    Q_SCRIPTABLE void next();
    Q_SCRIPTABLE void crop();
    Q_SCRIPTABLE Function findFunction(const QString &functionName) const;
    Q_SCRIPTABLE QStringList functions() const;
    Q_SCRIPTABLE QString lastError() const;
    Q_SCRIPTABLE QStringList tags(const QString &filename) const;

    Q_SCRIPTABLE inline bool load(const QString &path) { return load(QUrl(path), false); }
    Q_SCRIPTABLE inline bool loadRecursively(const QString &path) { return load(QUrl(path), true); }
    Q_SCRIPTABLE bool removeTracks(const QList<int> &tracks);
    Q_SCRIPTABLE bool removeTracks(int index, int count);
    Q_SCRIPTABLE bool removeTrack(int index) { return removeTracks(index, 1); }
    Q_SCRIPTABLE bool swapTrack(int from, int to);
    Q_SCRIPTABLE bool moveTrack(int from, int to);

    Q_SCRIPTABLE bool shuffle() const { return d.shuffle; }
    Q_SCRIPTABLE bool toggleShuffle() { setShuffle(!d.shuffle); return d.shuffle; }
    Q_SCRIPTABLE void setShuffle(bool on) { d.shuffle = on; }
signals:
    Q_SCRIPTABLE void wakeUp();
    Q_SCRIPTABLE void trackNames(int from, const QStringList &list);
    Q_SCRIPTABLE void currentTrackChanged(int index);

    Q_SCRIPTABLE void tracksInserted(int from, int count);
    Q_SCRIPTABLE void tracksChanged(int from, int count);
    Q_SCRIPTABLE void tracksRemoved(int from, int count);
    Q_SCRIPTABLE void trackMoved(int from, int to);
    Q_SCRIPTABLE void tracksSwapped(int from, int to);
    // need to emit this if e.g. the command line client changes the
    // volume on us. The other clients need to know to move their
    // slider etc
    Q_SCRIPTABLE void event(int type, const QList<QVariant> &data);
    Q_SCRIPTABLE void statusChanged(int status);
    Q_SCRIPTABLE void foo(int);
private slots:
#ifdef THREADED_RECURSIVE_LOAD
    void onThreadFinished();
#endif
#ifdef Q_OS_UNIX
    void onUnixSignal(int signal);
#endif
protected:
//    enum SyncMode { ToFile, FromFile };
    bool syncToFile();
    bool syncFromFile(bool *foundInvalidSongs);
//    bool sync(SyncMode sync, bool *removedSongs);
    enum RepeatMode { NoRepeat, RepeatOne, RepeatAll };
    void addTracks(const QStringList &list);
    struct Data {
        Data() : current(-1), root(0), blockSync(false), backend(0), shuffle(false), repeat(NoRepeat) {}
        int current;
        QFile playlist;
        QList<QUrl> tracks;
        QMap<QUrl, TrackData> cache;
        mutable FunctionNode *root;
        bool blockSync;
        Backend *backend;
        QList<TagInterface*> tagInterfaces;
        bool shuffle;
        RepeatMode repeat;
        mutable QString lastError;
    } d;
};

#endif
