#include "widgets.h"
#include "config.h"

SliderStyle::SliderStyle()
    : QWindowsStyle()
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
    object->render(p);
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
    case PM_SliderLength: return normal.targetRect.width();
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
        pixmaps[i].render(&p);
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
