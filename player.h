#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui>
class TokoloshInterface;

class SubPixmap : public QPixmap
{
public:
    SubPixmap() {}
    SubPixmap(const QString &path, const QRect &sub)
        : QPixmap(path), subRect(sub)
    {
        if (sub.isEmpty()
            sub = rect();
    }

    SubPixmap(const QPixmap &pix, const QRect &sub)
        : QPixmap(pix), subRect(sub)
    {
        if (sub.isEmpty()
            sub = rect();
    }

    void render(QPainter *p, const QPoint &pos)
    {
        p->drawPixmap(pos, *this, subRect);
    }
    void render(QPainter *p, const QRect &rect)
    {
        p->drawPixmap(rect, *this, subRect);
    }

private:
    QRect subRect;
};

class Button : public QAbstractButton
{
    Q_OBJECT
public:
    Button(QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);
private:
    enum { Normal = 0x0,
           Pressed = 0x1,
           Checked = 0x2,
           NumStates = 4
    };
    SubPixmap pixmaps[NumStates];
    friend class Player;
};

class Player : public QWidget
{
    Q_OBJECT
public:
    Player(QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    bool setSkin(const QString &path);
    void showEvent(QShowEvent *e);
private:
    enum ButtonType {
        Previous = 0,
        Play,
        Pause,
        Next,
        Stop,
        Open,
        OpenSkin,
        Shuffle,
        Repeat,
        Equalizer,
        Playlist,
        ButtonCount
    };

    enum ElementType {
        Stereo,
        Mono,
        ElementCount
    };

    struct Element {
        SubPixmap pixmap;
        QRect geometry;
    };

    struct Private {
        SubPixmap main;
        Button *buttons[ButtonCount];
        enum ChannelMode { Stereo, Mono } channelMode;
        Element elements[ElementCount];
        TokoloshInterface *tokolosh;
        QPoint dragOffset;
    } d;
};

#endif
