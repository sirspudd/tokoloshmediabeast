Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui>
#include <config.h>

struct RenderObject
{
    RenderObject() {}
    RenderObject(const QRect &tr, const QPixmap &pix, const QRect &sr)
        : pixmap(pix), targetRect(tr), sourceRect(sr)
    {}
    void render(QPainter *p) const // ### pass in clip?
    {
        static const bool smoothPixmapTransform = Config::isEnabled("smoothpixmaptransform", true);
        p->setRenderHints(smoothPixmapTransform ? QPainter::SmoothPixmapTransform : static_cast<QPainter::RenderHint>(0));
        const QRect sr = sourceRect.isNull() ? pixmap.rect() : sourceRect;
        const QRect tr = targetRect.isNull() ? QRect(QPoint(), sr.size()) : targetRect;
        p->drawPixmap(tr, pixmap, sr);
    }

    void render(QPainter *p, const QRect &tr) const // ### pass in clip?
    {
        static const bool smoothPixmapTransform = Config::isEnabled("smoothpixmaptransform", true);
        p->setRenderHints(smoothPixmapTransform ? QPainter::SmoothPixmapTransform : static_cast<QPainter::RenderHint>(0));
        const QRect sr = sourceRect.isNull() ? pixmap.rect() : sourceRect;
        p->drawPixmap(tr, pixmap, sr);
    }

    QPixmap pixmap;
    QRect targetRect, sourceRect;
};

class TextObject
{
public:
    TextObject()
    {}

    void render(QPainter *p, const QPoint &pos, const QString &string)
    {
        QPoint pp = pos;
        for (int i=0; i<string.size(); ++i) {
            const QRect r = sourceRects.value(string.at(i));
            if (r.isNull()) {
                qDebug() << string << string.at(i);
            }
            Q_ASSERT(!r.isNull());
            p->drawPixmap(pp, pixmap, r);
            pixmap.copy(r).save(string.at(i) + QLatin1String(".png"), "PNG");
            pp += QPoint(r.width(), 0);
        }
    }

    QPixmap pixmap;
    QHash<QChar, QRect> sourceRects;
};

#ifdef QT_DEBUG
class Overlay;
#endif
class QDBusInterface;
class WidgetResizer;
class Slider;
class SliderStyle;
class Button;
class TrackModel;
class PlaylistWidget;
class Player : public QWidget
{
    Q_OBJECT
public:
    Player(QDBusInterface *interface, QWidget *parent = 0);
    ~Player();
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
    QSize sizeHint() const;
    static bool verifySkin(const QString &dir);
public slots:
    void restoreDefaultSize();
    bool setSkin(const QString &path);
    void open();
    void openSkin();
    void reloadSettings();
    void editShortcuts();
    void wakeUp();
    void togglePlaylist();
#ifdef QT_DEBUG
    void debugButton();
    void toggleOverlay(bool on);
    void toggleDebugGeometry(bool on);
#endif
private:
    enum ActionType {
        Previous = 0,
        Play,
        Pause,
        Stop,
        Next,
        Open,
        Shuffle,
        Repeat,
        Equalizer,
        Playlist,
        ButtonCount,
        OpenSkin,
        ActionCount
    };

    enum ElementType {
        Stereo,
        Mono,
        ElementCount
    };

    struct Private {
        RenderObject main;
        TrackModel *model;
        PlaylistWidget *playlist;
        Button *buttons[ButtonCount];
        enum ChannelMode { Stereo, Mono } channelMode;
        RenderObject elements[ElementCount];
        QDBusInterface *interface;
        TextObject numbers, numbersEx, text;
        QPoint dragOffset;
        Slider *posBarSlider;
        SliderStyle *posBarStyle;
        Slider *volumeSlider;
        SliderStyle *volumeStyle;
        WidgetResizer *resizer;
        bool moving;
#ifdef QT_DEBUG
        Overlay *overlay;
#endif
    } d;
};

#endif
