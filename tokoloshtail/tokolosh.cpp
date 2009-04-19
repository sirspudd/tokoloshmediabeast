#include <tokolosh.h>

#include "tokolosh.h"

#include <QtCore>
#include <xine.h>
#include <xine/xineutils.h>
#include <math.h>

#include <qdebug.h>

class Tokolosh::Private : public QObject
{
    Q_OBJECT

public:
    Private(Tokolosh *pTokolosh);
    ~Private();
public slots:
    void pollResponse();
public:
    QStringList recursiveTrackLocate(const QDir &currentDir);
    int randomTrack();
    int normalizeTrackPosition(int pos);

    //For some reason having these static fucked things up
    xine_t *xine;
    xine_stream_t *stream;
    xine_audio_port_t *ao_port;
    xine_event_queue_t *event_queue;
    QString filePath;
    QStringList libContents;
    QStringList libPaths;
    int trackPosition;
    QStringList extensions;
    QTimer *xinePollingTimer;
    bool playing;
    bool shuffle;
    bool repeat;
    Tokolosh *pSelf;
};

Tokolosh::Private::Private(Tokolosh *pTokolosh)
    : QObject(pTokolosh),
      xine(0),
      stream(0),
      ao_port(0),
      event_queue(0),
      trackPosition(0),
      playing(false),
      shuffle(false),
      repeat(false),
      pSelf(pTokolosh)
{
    //Xine initialization
    char configfile[2048];

    xine = xine_new();
    sprintf(configfile, "%s%s", xine_get_homedir(), "/.xine/config");
    xine_config_load(xine, configfile);
    xine_init(xine);

    ao_port = xine_open_audio_driver(xine , "auto" , NULL);
    stream = xine_stream_new(xine, ao_port, NULL);
    event_queue = xine_event_new_queue(stream);

    //Container init
    //Global const complex variables, initialization can be a killer

    xinePollingTimer = new QTimer(this);
    //Not too damn taxing
    xinePollingTimer->setInterval(500);

    connect(xinePollingTimer,SIGNAL(timeout()),this,SLOT(pollResponse()));
    extensions << "*.mp3";
    extensions << "*.ogg";
    // qDBusRegisterMetaType<QStringList>();
}

Tokolosh::Private::~Private()
{
    xine_close(stream);
    xine_event_dispose_queue(event_queue);
    xine_dispose(stream);
    if (ao_port)
        xine_close_audio_driver(xine, ao_port);
    xine_exit(xine);
}

void Tokolosh::Private::pollResponse()
{
    //This is not running unless Xine is meant to be playing
    // qDebug() << "Polling";
    if (pSelf->getStatus() == XINE_STATUS_STOP)
        pSelf->next();
    // qDebug() << "Status : " << getStatus();
}

QStringList Tokolosh::Private::recursiveTrackLocate(const QDir &currentDir)
{ //Stuff I can reuse ad infinitum and which needs to go into some common personal headers
    if (currentDir.count() < 1) {
        return QStringList();
    }

    QStringList tempTrackList = currentDir.entryList(extensions,QDir::Files,QDir::Unsorted);
    QStringList qualifiedTrackList;

    for (int count = 0; count < tempTrackList.count();count++)
        qualifiedTrackList << ((currentDir.absolutePath() + "/" + tempTrackList[count]));

    QStringList tempDirList = currentDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot,QDir::Unsorted);

    for (int count = 0; count < tempDirList.count();count++) {
        const QDir bastard((currentDir.absolutePath() + "/" + tempDirList[count]));
        qualifiedTrackList << recursiveTrackLocate(bastard);
    }
    return qualifiedTrackList;
}

int Tokolosh::Private::randomTrack()
{
    return (rand() % libContents.count());
}

int Tokolosh::Private::normalizeTrackPosition(int pos)
{
    if ((pos >= 0) & (pos < libContents.size()))
        return pos;
    if (pos < 0)
        return (pos + libContents.size());
    else return (pos - libContents.size());
    //All f$%#ed up
    return -1;
}

Tokolosh::Tokolosh(QObject *parent)
    : QObject(parent),
      d(new Private(this))
{
}

Tokolosh::~Tokolosh()
{
    delete d;
    d=0;
}

void Tokolosh::load(const QString &fileName)
{
    if (!d->repeat)
        d->filePath = fileName;
}

void Tokolosh::play()
{
    //need to distinguish between stopped and idle
    if (!d->playing)
        d->playing = !d->playing;

    d->xinePollingTimer->start();
    if (d->filePath.isEmpty()) {
        if (d->libContents.isEmpty())
            stop();
        else
            next();
        return;
    }
    //Start

    xine_close(d->stream);

    if ((!xine_open(d->stream, d->filePath.toAscii().constData())) || (!xine_play(d->stream, 0, 0))) {
        printf("Unable to open mrl '%s'\n", d->filePath.toAscii().constData());
        // return 0;
        //emit signal here
    }
    emit trackChanged(d->filePath);
}

bool Tokolosh::shuffle() const
{
    return d->shuffle;
}

bool Tokolosh::repeat() const
{
    return d->shuffle;
}

QString Tokolosh::track() const
{
    return d->filePath;
}


void Tokolosh::stop()
{
    if (d->playing)
        d->playing = !d->playing;

    d->xinePollingTimer->stop();

    xine_close(d->stream);
    // if (!(xine_stop(stream))) {
    // printf("Unable to stop mrl '%s'\n", filePath.toAscii().constData());
    // }
}

QDBusVariant Tokolosh::playlistWindow(int window)
{
    QStringList contentWindow;
    // if (libContents.size() < window)
    if (d->libContents.size() < window)
        return QDBusVariant(d->libContents);
    int offset = round(window / 2);
    for (int trackNo = 0; trackNo < window; trackNo++)
        contentWindow.append(d->libContents.at(d->normalizeTrackPosition(d->trackPosition - offset + trackNo)));
    return QDBusVariant(contentWindow);
}

QDBusVariant Tokolosh::libraryContents()
{
    return QDBusVariant(d->libContents);
}
QDBusVariant Tokolosh::libraryPaths()
{
    return QDBusVariant(d->libPaths);
}

int Tokolosh::getStatus()
{
    //Already defined by xine : Just wrapping
    return xine_get_status(d->stream);
}

void Tokolosh::pause()
{
    if (xine_get_param(d->stream, XINE_PARAM_SPEED) != XINE_SPEED_PAUSE)
        xine_set_param(d->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
    else
        xine_set_param(d->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
}

void Tokolosh::toggleMute()
{
    if (xine_get_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE) != 1)
        xine_set_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE, 1);
    else
        xine_set_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE, 0);
}

int Tokolosh::volume() const
{
    return xine_get_param(d->stream, XINE_PARAM_AUDIO_VOLUME);
}

void Tokolosh::setVolume(int vol)
{
    xine_set_param(d->stream, XINE_PARAM_AUDIO_VOLUME, vol);
}

void Tokolosh::clearLibraryPaths()
{
    d->libPaths.clear();
}

void Tokolosh::searchedLibraryPaths()
{// Want these returned by DBUS eventually, avoiding QVariant goodness
    QStringListIterator musicDirs(d->libPaths);
    qDebug() << "Directories in the library";
    while (musicDirs.hasNext())
        qDebug() << musicDirs.next() << endl;
    qDebug() << "Directories in the library : end";
}

void Tokolosh::addLibraryPath(const QString &path)
{
    bool redundant = false;
    //Quick checks around whether the added dir is lower/higher than the existing entries
    qDebug() << "Redundant paths will be obliterated from the maintained list of paths";
    //Check for existing parent
    QStringListIterator musicDirs(d->libPaths);
    QString musicDir;

    while (musicDirs.hasNext()) { musicDir = musicDirs.next();
        if (musicDir.startsWith(path))
            d->libPaths.removeAt(d->libPaths.indexOf(musicDir));
    }
    musicDirs = QStringListIterator(d->libPaths);

    while (musicDirs.hasNext()) { musicDir = musicDirs.next();
        if (path.startsWith(musicDir))
            redundant = true;
    }

    if (!redundant) {
        d->libPaths << path;
        populateLibrary();
    } else qDebug() << "Suggested path is already recursively covered.";
}

void Tokolosh::removeLibraryPath(const QString &path)
{
    if (!d->libPaths.contains(path))
        return;
    d->libPaths.removeAt(d->libPaths.indexOf(path));
    populateLibrary();
}

void Tokolosh::populateLibrary()
{
    //init
    //clear library contents
    d->libContents.clear();
    // qDebug() << "So starts the madness";
    QStringListIterator musicDirs(d->libPaths);
    while (musicDirs.hasNext())
        d->libContents << d->recursiveTrackLocate(musicDirs.next());
    // qDebug() << "Hold on to your hat";
    qDebug() << d->libContents;
    d->trackPosition = d->normalizeTrackPosition(d->trackPosition);
}

void Tokolosh::next()
{
    if (d->libContents.count() != 0) {
        d->trackPosition = d->shuffle ? d->randomTrack() : d->normalizeTrackPosition(++d->trackPosition);
        load(d->libContents.at(d->trackPosition));
        qDebug() << getStatus();
        if (d->playing)
            play();
    }
}

void Tokolosh::prev()
{
    if (d->libContents.count() != 0) {
        d->trackPosition = d->shuffle ? d->randomTrack() : d->normalizeTrackPosition(--d->trackPosition);
        load(d->libContents.at(d->trackPosition));
        if (d->playing)
            play();
    }
}

void Tokolosh::toggleShuffle()
{
    qDebug("%s %d: void Tokolosh::toggleShuffle()", __FILE__, __LINE__);
    d->shuffle = !d->shuffle;
    emit shuffleChanged(d->shuffle);
}


void Tokolosh::setShuffle(bool on)
{
    if (on != d->shuffle)
        toggleShuffle();
}

void Tokolosh::setRepeat(bool on)
{
    if (on != d->repeat)
        toggleRepeat();
}


void Tokolosh::toggleRepeat()
{
    d->repeat = !d->repeat;
    emit repeatChanged(d->repeat);
}

void Tokolosh::syncClients()
{
    emit trackChanged(d->filePath);
    emit shuffleChanged(d->shuffle);
    emit repeatChanged(d->repeat);
}

int Tokolosh::speed()
{
    return xine_get_param(d->stream, XINE_PARAM_SPEED);
}

void Tokolosh::setSpeed(int speed)
{
    switch (speed) {
    case XINE_SPEED_PAUSE :
    case XINE_SPEED_SLOW_4 :
    case XINE_SPEED_SLOW_2 :
    case XINE_SPEED_NORMAL :
    case XINE_SPEED_FAST_2 :
    case XINE_SPEED_FAST_4 :
        xine_set_param(d->stream, XINE_PARAM_SPEED,speed);
    default:
        qDebug() << "Not defined";
    }
}

#include "tokolosh.moc"

/*
  Donald Carr(sirspudd_at_gmail.com) plays songs occasionally
  Copyright(C) 2007 Donald Car

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
