#ifndef SKIN_H
#define SKIN_H

#include <QtGui>
class SkinPrivate;
class TokoloshInterface;

class SkinButton : public QAbstractButton
{
    Q_OBJECT
public:
    SkinButton(QWidget *parent = 0);
    void setNormalPixmap(const QPixmap &pixmap, const QRect &source);
    void setDownPixmap(const QPixmap &pixmap, const QRect &source);
    void setHoverPixmap(const QPixmap &pixmap, const QRect &source);
    void paintEvent(QPaintEvent *e);
private:
    enum { Normal, Down, Hover };
    void setPixmap(int i, const QPixmap &pixmap, const QRect &source);
    struct Private {
        QPixmap pixmaps[3];
        QRect sourceRects[3];
    } d;
};

class Player : public QWidget
{
    Q_OBJECT
public:
    Player(QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void setSkin(const QString &path);
private:
    enum Button {
        Previous = 0,
        Play,
        Pause,
        Next,
        Stop,
        Open,
        OpenSkin,
        Shuffle,
        Repeat,
        Playlist,
        ButtonCount
    };

    struct Private {
        SkinButton *buttons[ButtonCount];
        QPixmap background;
        TokoloshInterface *tokolosh;
        QPoint dragOffset;
    } d;
};


#endif
