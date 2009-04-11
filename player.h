#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui>
class TokoloshInterface;

class Button : public QAbstractButton
{
    Q_OBJECT
public:
    Button(QWidget *parent = 0);
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
        Playlist,
        ButtonCount
    };

    struct Private {
        Button *buttons[ButtonCount];
        QPixmap main;
        TokoloshInterface *tokolosh;
        QPoint dragOffset;
    } d;
};

#endif
