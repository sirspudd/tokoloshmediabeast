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

#ifndef PHONONBACKEND_H
#define PHONONBACKEND_H

#include <QtCore>
#include <phonon>
#include "backend.h"
#include "backendplugin.h"

struct Private;
class Q_DECL_EXPORT PhononBackend : public Backend
{
public:
    PhononBackend(QObject *tail);
    virtual ~PhononBackend();
    virtual bool initBackend();
    virtual void shutdown();
    virtual bool trackData(TrackData *data, const QUrl &url, int types = All) const;
    virtual bool isValid(const QUrl &url) const;
    virtual void play();
    virtual void pause();
    virtual void setProgress(int type, int progress);
    virtual int progress(int type);
    virtual void stop();
    virtual bool loadUrl(const QUrl &url);
    virtual int status() const;
    virtual int volume() const;
    virtual void setVolume(int vol);
    virtual QString errorMessage() const;
    virtual int errorCode() const;
    virtual void setMute(bool on);
    virtual bool isMute() const;
    virtual int flags() const;
private:
    Private *d;
};

class Q_DECL_EXPORT PhononBackendPlugin : public BackendPlugin
{
public:
    PhononBackendPlugin() : BackendPlugin(QStringList() << "phonon" << "phononbackend") {}
    virtual Backend *createBackend(QObject *parent)
    {
        return new PhononBackend(parent);
    }

};


#endif
