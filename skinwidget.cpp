#include "skinwidget.h"
#include "player.h"

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QApplication>
#include <QDir>
#include <qdebug.h>
#include <QImageReader>

class SkinWidget;
class Player;

class WinampSkin
{
public:
    bool sanityCheck() {
        return !(baseSkin.isNull() | baseButtons.isNull() | rsButtons.isNull());
    }

    QString path;
    QImage baseSkin;
    QImage baseButtons;
    QImage rsButtons;
};

class SkinWidget::Private
{
public:
    Private(SkinWidget *parent);

    void stripSkin(QString &path);

    WinampSkin currentSkin;
    int currentInterface;

    //Skin
    int winampSkinState;

    QString trackName;

    bool shuffle;
    bool repeat;
    bool equalizer;
    bool playlist;

    Player *playerWidget;
    QStringList supportedImageFormats;
    SkinWidget *pSelf;
};

SkinWidget::Private::Private(SkinWidget *parent)
    : winampSkinState(-1),
      shuffle(false),
      repeat(false),
      equalizer(false),
      playlist(false),
      playerWidget(0),
      pSelf(parent)
{
// qDebug() << QImageReader::supportedImageFormats();
    foreach(const QByteArray &imageFormatExtension, QImageReader::supportedImageFormats()) {
        supportedImageFormats << "*" + imageFormatExtension;
    }
}

void SkinWidget::Private::stripSkin(QString &path)
{
    //Case sensitivity might be broken : yetch
    WinampSkin newSkin;
    newSkin.path = QString(path);

    QDir testSkin(path);

    QStringList skinFiles = testSkin.entryList(supportedImageFormats, QDir::Files);
    QStringListIterator skinFileTrawl(skinFiles);

    while (skinFileTrawl.hasNext()) {
        QString tempString = skinFileTrawl.next();
        QString lowerString = tempString.toLower();
        if (lowerString.contains("main"))
            newSkin.baseSkin = QImage((path + "/" + tempString));
        if (lowerString.contains("cbuttons"))
            newSkin.baseButtons = QImage((path + "/" + tempString));
        if (lowerString.contains("shufrep"))
            newSkin.rsButtons = QImage((path + "/" + tempString));
    }

    if (newSkin.sanityCheck())
        currentSkin = WinampSkin(newSkin);

    pSelf->update();
}

SkinWidget::SkinWidget(GUI interface, QWidget* player)
    : QWidget(player),
      d(new Private(this))
{
    d->currentInterface = interface;
    d->playerWidget = static_cast<Player*>(player);
    //Sets default skin path
    setSkinPath((QApplication::applicationDirPath() + "/skins/YummiYogurt"));
}

SkinWidget::~SkinWidget()
{
    delete d;
    d = 0;
}

void SkinWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    switch (d->currentInterface) {
    case Designer:
        QWidget::paintEvent(e);
        break;
    case Winamp:
        //base skin
        painter.drawImage(e->rect(),d->currentSkin.baseSkin);
        //trackName
        painter.drawText(QRect(120,27,140,15),d->trackName);
        //base buttons
        painter.drawImage(QRect(16,86,114,18),d->currentSkin.baseButtons,QRect(0,0,114,18));
        //open
        painter.drawImage(QRect(16 + 114 + 6 /*magic offset*/,87,22,16),d->currentSkin.baseButtons,QRect(114,0,22,16));
        //playlist eq
        painter.drawImage(QRect(219,57,46,13),d->currentSkin.rsButtons,QRect(0,61,46,13));

        if (d->equalizer)
            painter.drawImage(QRect(219,57,23,13),d->currentSkin.rsButtons,QRect(0,74,23,13));
        if (d->playlist)
            painter.drawImage(QRect(242,57,23,13),d->currentSkin.rsButtons,QRect(23,74,23,13));
        if (d->repeat)
            painter.drawImage(QRect(209,87,28,15),d->currentSkin.rsButtons,QRect(0,30,28,15));
        else
            painter.drawImage(QRect(209,87,28,15),d->currentSkin.rsButtons,QRect(0,0,28,15));

        if (d->shuffle)
            painter.drawImage(QRect(164,87,43,15),d->currentSkin.rsButtons,QRect(28,30,43,15));
        else
            painter.drawImage(QRect(164,87,43,15),d->currentSkin.rsButtons,QRect(28,0,43,15));

        if (d->winampSkinState != -1) { //Allowing for 1 button visibly pressed at any point
            if (d->winampSkinState > -1 & d->winampSkinState < 5)
                painter.drawImage(QRect(16 + 23*d->winampSkinState,86,22,18),d->currentSkin.baseButtons,QRect(23*d->winampSkinState,18,22,18));
            if (d->winampSkinState == 5)
                painter.drawImage(QRect(16 + 114 + 6 /*magic offset*/,87,22,16),d->currentSkin.baseButtons,QRect(114,16,22,16));
        }
        break;
    default:
        break;
    }
    painter.end();
}

void SkinWidget::setTrackName(QString title)
{
    d->trackName = title.right(title.size() - (title.lastIndexOf("/") + 1));
    qDebug() << d->trackName;
}

void SkinWidget::setSkinPath(QString path)
{
    d->stripSkin(path);
    emit skinChanged();
}

void SkinWidget::mousePressEvent(QMouseEvent * e)
{
// qDebug() << e->pos();
    switch (d->currentInterface) {
    case Designer :
        QWidget::mousePressEvent(e);
        break;
    case Winamp :
        //Basic buttons
        for (int count = 0; count < 5; count++)
            if (QRect(16 + count*22,86,22,18).contains(e->pos())) {
                d->winampSkinState = count;
                break;
            }

        //Open file button
        if (QRect(136,87,22,16).contains(e->pos()))
            d->winampSkinState = 5;

        //Change skin button : no state
        if (QRect(246,84,30,20).contains(e->pos()))
            emit openSkin();

        if (QRect(209,86,27,20).contains(e->pos())) {
            emit repeat();
        }

        if (QRect(164,86,45,20).contains(e->pos())) {
            emit shuffle();
        }

        if (QRect(219,57,23,13).contains(e->pos())) {
            d->equalizer = !d->equalizer;
        }

        if (QRect(242,57,23,13).contains(e->pos())) {
            d->playlist = !d->playlist;
            //if (playerWidget->playlistVisible()) playerWidget->hidePlaylist();
            //else playerWidget->showPlaylist();
        }

        // qDebug() << winampSkinState;

        switch (d->winampSkinState) {//These numbers are completely magical(Me decoding Winamp theme files)
        case 0 :
            emit prev();
            break;
        case 1 :
            emit play();
            break;
        case 2 :
            emit pause();
            break;
        case 3 :
            emit stop();
            break;
        case 4 :
            emit next();
            break;
        case 5 :
            emit open();
            break;
        default:
            break;
        }

        break;

    default :
        break;
    }
    update();
}

void SkinWidget::mouseReleaseEvent(QMouseEvent * e)
{
    d->winampSkinState = -1;
    update();
}

void SkinWidget::setShuffleState(bool shuffleState)
{
    d->shuffle = shuffleState;
}

void SkinWidget::setRepeatState(bool repeatState)
{
    d->repeat = repeatState;
}

QSize SkinWidget::sizeHint() const
{
    switch (d->currentInterface) {
    case Designer:
        return QWidget::sizeHint();
        //I know I know colour me paranoid
        break;
    case Winamp:
        return QSize(275,114);
    default :
        return QSize(0,0);
    }
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
