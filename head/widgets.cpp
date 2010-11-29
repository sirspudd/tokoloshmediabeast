Copyright (c) 2010 Anders Bakken
Copyright (c) 2010 Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "widgets.h"
#include "config.h"

SliderStyle::SliderStyle(Player *p)
    : QWindowsStyle(), player(p)
{
}

void SliderStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                     QPainter *p, const QWidget *) const
{
    Q_ASSERT(cc == CC_Slider);
    Q_UNUSED(cc);
    const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider*>(opt);
    RenderObject *object = (slider->state & State_Sunken
                            && slider->activeSubControls == SC_SliderHandle ? &pressed : &normal);
    if (object == &pressed) {
        groovePressed.render(p);
    }
    QRect &r = object->targetRect;
    const double w = slider->rect.width() - r.width();
    const double x = double(slider->sliderValue - slider->minimum) / double(slider->maximum - slider->minimum) * w;

    r.moveLeft(int(x));
    p->save();
    p->setTransform(qVariantValue<QTransform>(player->property("transform")));
    object->render(p);
    p->restore();
}

int SliderStyle::styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget,
                           QStyleHintReturn* returnData) const
{
    if (stylehint == SH_Slider_AbsoluteSetButtons) {
        return qVariantValue<int>(property("SH_Slider_AbsoluteSetButtons"));
    } else {
        return QWindowsStyle::styleHint(stylehint, opt, widget, returnData);
    }
}

int SliderStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    switch (m) {
    case PM_SliderLength:
        return qVariantValue<QTransform>(player->property("transform")).mapRect(normal.targetRect).width();
//    case PM_MaximumDragDistance: return INT_MAX;
    default: break;
    }
    return QWindowsStyle::pixelMetric(m, opt, widget);
}

Button::Button(QWidget *parent)
    : QAbstractButton(parent)
{
}

void Button::paintEvent(QPaintEvent *)
{
    // ### use paintEvent->rect() ?
    int i = isChecked() ? Checked : Normal;
    if (isDown() && !pixmaps[i | Pressed].pixmap.isNull())
        i |= Pressed;
    if (!pixmaps[i].pixmap.isNull()) {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setTransform(qVariantValue<QTransform>(window()->property("transform")));
        pixmaps[i].render(&p);
//        Q_ASSERT(pixmaps[i].sourceRect.isNull() || pixmaps[i].sourceRect.size() == size());
#ifdef QT_DEBUG
        if (Config::isEnabled("debuggeometry", false)) {
            p.setPen(Qt::white);
            p.drawRect(0, 0, width() - 1, height() - 1);
        }
#endif
    }
}

Slider::Slider(Qt::Orientation o, QWidget *parent)
    : QSlider(o, parent)
{
}

void Slider::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QStyleOptionSlider slider;
        initStyleOption(&slider);
        if (style()->subControlRect(QStyle::CC_Slider, &slider, QStyle::SC_SliderHandle, this)
            .contains(e->pos())) {
            style()->setProperty("SH_Slider_AbsoluteSetButtons", Qt::NoButton);
            update();
        } else {
            style()->setProperty("SH_Slider_AbsoluteSetButtons", Qt::LeftButton);
        }
    }

    QSlider::mousePressEvent(e);
    e->accept();
}

void Slider::mouseMoveEvent(QMouseEvent *e)
{
    const QRect r = rect();
    QMouseEvent me(e->type(), QPoint(qBound(r.left(), e->x(), r.right()),
                                     qBound(r.top(), e->y(), r.bottom())),
                   e->button(), e->buttons(), e->modifiers());
    QSlider::mouseMoveEvent(&me); // don't want to snap back
    e->accept();
}

void Slider::mouseReleaseEvent(QMouseEvent *e)
{
    QSlider::mouseReleaseEvent(e);
    e->accept();
    update();
}

