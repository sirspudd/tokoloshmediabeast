#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui>
class TokoloshInterface;

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
    struct Private {
        QPixmap pixmaps[NumStates];
        QRect sourceRects[NumStates];
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
        Equalizer,
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
