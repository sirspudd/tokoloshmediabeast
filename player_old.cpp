#include "player.h"

#include "skinwidget.h"

//generated
#include "ui_player.h"
#include "tokolosh_interface.h"

#include <QFileDialog>
#include <QSystemTrayIcon>
#include <QMenu>

class PlaylistWidget;

class Player::Private : public QObject
{
    Q_OBJECT
public:
    Private(Player *parent);
    ~Private();
    void setDesignerUI();
    void setWinampUI();

    QString appPath;
    TokoloshInterface *tokolosh;

    QSystemTrayIcon *trayIcon;
    QMenu *contextMenu;

    Ui_Player *designerDec;

    QAction *playAction;
    QAction *pauseAction;
    QAction *stopAction;
    QAction *nextAction;
    QAction *prevAction;
    QAction *volUpAction;
    QAction *volDownAction;
    QAction *toggleShuffleAction;
    QAction *toggleRepeatAction;
    QAction *loadSongAction;
    QAction *loadSkinAction;

    int volume;
    GUI currentInterface;
    SkinWidget *guiWidget;

public slots:
    void loadSong();
    void loadSkin();
    void updateActions();
    void updateTrackDetails(QString trackTitle);
    void setShuffleState(bool shuffleState);
    void setRepeatState(bool repeatState);
    void showPlaylist();
    void hidePlaylist();
    void increaseVolume();
    void decreaseVolume();
signals:
    void anouncePresence();
};

Player::Private::Private(Player *parent)
    : QObject(parent),
      appPath(QApplication::applicationDirPath()),
      tokolosh(new TokoloshInterface("com.TokoloshXineBackend.TokoloshMediaPlayer", "/TokoloshMediaPlayer",QDBusConnection::sessionBus(), this)),
      trayIcon(new QSystemTrayIcon(QIcon(appPath + "/generic.png"),this)),
      contextMenu(new QMenu(0)),
      designerDec(0),
      playAction(new QAction("Play",this)),
      pauseAction(new QAction("Pause",this)),
      stopAction(new QAction("Stop",this)),
      nextAction(new QAction("Next Track",this)),
      prevAction(new QAction("Previous Track",this)),
      volUpAction(new QAction("Volume Up",this)),
      volDownAction(new QAction("volume Down",this)),
      toggleShuffleAction(new QAction("Shuffle",this)),
      toggleRepeatAction(new QAction("Repeat",this)),
      loadSongAction(new QAction("Load song",this)),
      loadSkinAction(new QAction("Load skin",this)),
      volume(50),
      currentInterface(Winamp),
      guiWidget(new SkinWidget(currentInterface, parent))
{
    parent->setCentralWidget(guiWidget);

    //QMenu populate
    contextMenu->addAction(playAction);
    contextMenu->addAction(pauseAction);
    contextMenu->addAction(stopAction);
    contextMenu->addAction(nextAction);
    contextMenu->addAction(prevAction);
    contextMenu->addAction(volUpAction);
    contextMenu->addAction(volDownAction);
    contextMenu->addAction(toggleShuffleAction);
    contextMenu->addAction(toggleRepeatAction);
    contextMenu->addAction(loadSkinAction);
    contextMenu->addAction(loadSongAction);

    //Connect actions to backend
    connect(playAction,SIGNAL(triggered(bool)),tokolosh,SLOT(play()));
    connect(pauseAction,SIGNAL(triggered(bool)),tokolosh,SLOT(pause()));
    connect(stopAction,SIGNAL(triggered(bool)),tokolosh,SLOT(stop()));
    connect(prevAction,SIGNAL(triggered(bool)),tokolosh,SLOT(prev()));
    connect(nextAction,SIGNAL(triggered(bool)),tokolosh,SLOT(next()));
    connect(volUpAction,SIGNAL(triggered(bool)),SLOT(increaseVolume()));
    connect(volDownAction,SIGNAL(triggered(bool)),SLOT(decreaseVolume()));
    connect(toggleShuffleAction,SIGNAL(triggered(bool)),tokolosh,SLOT(toggleShuffle()));
    connect(toggleRepeatAction,SIGNAL(triggered(bool)),tokolosh,SLOT(toggleRepeat()));

    connect(loadSkinAction,SIGNAL(triggered(bool)),this,SLOT(loadSkin()));
    connect(loadSongAction,SIGNAL(triggered(bool)),this,SLOT(loadSong()));

    //Notify tokolosh daemon of arrival
    connect(this,SIGNAL(anouncePresence()),tokolosh,SLOT(syncClients()));

    trayIcon->setContextMenu(contextMenu);
    trayIcon->show();

    switch (currentInterface) {
    case Designer :
        setDesignerUI();
        break;
    case Winamp :
        setWinampUI();
        break;
    default:
        break;
    }

    connect(tokolosh,SIGNAL(trackChanged(QString)),this,SLOT(updateTrackDetails(QString)));
    connect(tokolosh,SIGNAL(shuffle(bool)),this,SLOT(setShuffleState(bool)));
    connect(tokolosh,SIGNAL(repeat(bool)),this,SLOT(setRepeatState(bool)));

    emit anouncePresence();
}

Player::Private::~Private()
{
    stopAction->trigger();
}

void Player::Private::showPlaylist()
{
}

void Player::Private::hidePlaylist()
{
}

void Player::Private::increaseVolume() {
    tokolosh->setVolume(100);
}

void Player::Private::decreaseVolume() {
    tokolosh->setVolume(0);
}

void Player::Private::setShuffleState(bool enable)
{
    guiWidget->setShuffleState(enable);
    guiWidget->update();
}

void Player::Private::setRepeatState(bool enable)
{
    guiWidget->setRepeatState(enable);
    guiWidget->update();
}

void Player::Private::updateTrackDetails(const QString trackString)
{
    switch (currentInterface) {
    case Designer :
        designerDec->trackName->setText(trackString);
        break;
    case Winamp :
        guiWidget->setTrackName(trackString);
        break;
    default :
        break;
    }

    guiWidget->update();

    trayIcon->setToolTip(trackString);
    trayIcon->showMessage("Din :", trackString,QSystemTrayIcon::Information,3000);
}

void Player::Private::setDesignerUI()
{
    if (!designerDec) {
        designerDec = new Ui_Player();

        designerDec->setupUi(guiWidget);

        //Connect ui to actions
        connect(designerDec->playButton,SIGNAL(clicked(bool)),playAction,SLOT(trigger()));
        connect(designerDec->pauseButton,SIGNAL(clicked(bool)),pauseAction,SLOT(trigger()));
        connect(designerDec->stopButton,SIGNAL(clicked(bool)),stopAction,SLOT(trigger()));
        connect(designerDec->prevButton,SIGNAL(clicked(bool)),prevAction,SLOT(trigger()));
        connect(designerDec->nextButton,SIGNAL(clicked(bool)),nextAction,SLOT(trigger()));
        connect(designerDec->volUpButton,SIGNAL(clicked(bool)),volUpAction,SLOT(trigger()));
        connect(designerDec->volDownButton,SIGNAL(clicked(bool)),volDownAction,SLOT(trigger()));

        connect(designerDec->openButton,SIGNAL(clicked(bool)),this,SLOT(loadSong()));
    }
}

void Player::Private::setWinampUI()
{
    connect(guiWidget,SIGNAL(play()),playAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(pause()),pauseAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(stop()),stopAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(prev()),prevAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(next()),nextAction,SLOT(trigger()));
    //connect(guiWidget,SIGNAL(volUp()),volUpAction,SLOT(trigger()));
    //connect(guiWidget,SIGNAL(volDown()),volDownAction,SLOT(trigger()));

    connect(guiWidget,SIGNAL(shuffle()),toggleShuffleAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(repeat()),toggleRepeatAction,SLOT(trigger()));

    connect(guiWidget,SIGNAL(open()),loadSongAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(openSkin()),loadSkinAction,SLOT(trigger()));
    connect(guiWidget,SIGNAL(skinChange()),this,SLOT(updateActions()));
}

void Player::Private::updateActions()
{
    //Parse out buttons and set them as icons on the appropriate actions : bleh
}

void Player::Private::loadSong()
{
    tokolosh->load(QFileDialog::getOpenFileName(0,"Choose your poison","/mnt/sod/music"));
}

void Player::Private::loadSkin()
{
    QString fucker = QFileDialog::getExistingDirectory(0,"Pick your spots");
    qDebug() << "Setting skin to :" << fucker;
    guiWidget->setSkinPath(fucker);
}

Player::Player(QWidget *parent)
  : QWidget(parent),
    d(new Private(this))
{
    setFixedSize(sizeHint());
}

Player::~Player()
{
    delete d;
    d=0;
}

/*
  Donald Carr(sirspudd_at_gmail.com) plays songs occasionally
  Copyright(C) 2007 Donald Car

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNES FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MAction 02110-1301 USAction.
*/

#include "player.moc"
