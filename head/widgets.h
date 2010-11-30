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

#ifndef WIDGETS_H
#define WIDGETS_H

#include "player.h"
#include <QtGui>

class SliderStyle : public QWindowsStyle
{
public:
    SliderStyle(Player *player);
    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                    const QWidget *widget = 0) const;
    virtual int pixelMetric(PixelMetric m, const QStyleOption *opt = 0, const QWidget *widget = 0) const;
    virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                          const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
private:
    Player *player;
    friend class Player;
    mutable RenderObject normal, pressed;
    RenderObject groovePressed;
};

class Slider : public QSlider
{
public:
    Slider(Qt::Orientation o, QWidget *parent);
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
