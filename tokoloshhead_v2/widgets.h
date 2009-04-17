#ifndef WIDGETS_H
#define WIDGETS_H

#include "player.h"
#include <QtGui>

class PosBarSliderStyle : public QWindowsStyle
{
public:
    PosBarSliderStyle();
    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                    const QWidget *widget = 0) const;
    virtual int pixelMetric(PixelMetric m, const QStyleOption *opt = 0, const QWidget *widget = 0) const;
    virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                          const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
private:
    friend class Player;
    mutable RenderObject normal, pressed;
    RenderObject groovePressed;
};

class PosBarSlider : public QSlider
{
public:
    PosBarSlider(Qt::Orientation o, QWidget *parent);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
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
    RenderObject pixmaps[NumStates];
    friend class Player;
};

#endif
