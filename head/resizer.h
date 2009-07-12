#ifndef RESIZER_H
#define RESIZER_H

#include <QRectF>
#include <QList>
#include <QPainter>

template<class Point, class Size, class Rect>
class AbstractResizer
{
public:
    enum Area {
        NoArea = 0x000,
        TopLeft = 0x001,
        Left = 0x002,
        BottomLeft = 0x004,
        Bottom = 0x008,
        BottomRight = 0x010,
        Right = 0x020,
        TopRight = 0x040,
        Top = 0x080,
        Center = 0x100,
        AllAreas = (TopLeft|Left|BottomLeft|Bottom|BottomRight|Right|TopRight|Top|Center)
    };

    class ResizerStyleOption
    {
    public:
        ResizerStyleOption() : area(AbstractResizer::NoArea), state(None) {}
        Rect rect;
        Area area;
        enum State {
            None = 0x0,
            Active = 0x1,
            Hovered = 0x2
        };

        uint state;
    };

    AbstractResizer()
        : d(new Private)
    {
        d->areas = AllAreas;
        d->hoveredArea = d->activeArea = NoArea;
        d->enabled = true;
        d->cursorSet = false;
    }

    virtual ~AbstractResizer()
    {
        delete d;
    }

    virtual bool handleMousePress(const Point &pos, Qt::MouseButton button);
    virtual bool handleMouseMove(const Point &pos, Qt::MouseButtons buttons);
    virtual bool handleMouseRelease(const Point &pos, Qt::MouseButton button);

    virtual Rect areaRect(Area a) const;
    virtual void paint(QPainter *p, const Rect &clip = Rect());
    virtual void paintArea(QPainter *p, const ResizerStyleOption *opt);
    virtual void update(const Rect &rect = Rect()) = 0;
    virtual void setGeometry(const Rect &rect) = 0;
    virtual Rect geometry() const = 0;
    virtual QCursor cursorForArea(Area a) const;
    virtual Size areaSize(Area a) const;

    virtual Area areaAt(const Point &pos) const
    {
        Area area = TopLeft;
        while (area <= Center) {
            if (d->areas & area && areaRect(area).contains(pos)) {
                return area;
            }
            area = Area(area << 1);
        }
        if (d->areas & Left && areaRect(Left).right() >= pos.x()) {
            return Left;
        } else if (d->areas & Right && areaRect(Right).left() <= pos.x()) {
            return Right;
        } if (d->areas & Top && areaRect(Top).bottom() >= pos.y()) {
            return Top;
        } else if (d->areas & Bottom && areaRect(Bottom).top() <= pos.y()) {
            return Bottom;
        }

        return NoArea;
    }

    uint areas() const { return d->areas; }
    void setAreas(uint a)
    {
        d->areas = a;
        bool unset = false;
        if (!(d->activeArea & a)) {
            d->activeArea = NoArea;
            unset = true;
        }
        if (!(d->hoveredArea & a)) {
            d->hoveredArea = NoArea;
            unset = true;
        }
        if (unset && d->cursorSet) {
            qApp->restoreOverrideCursor();
            d->cursorSet = false;
        }
        update();
    }

    Area hoveredArea() const { return d->hoveredArea; }
    void setHoveredArea(Area a)
    {
        if (d->hoveredArea != a) {
            Rect r;
            if (d->cursorSet) {
                qApp->restoreOverrideCursor();
                d->cursorSet = false;
            }

            if (d->hoveredArea != NoArea) {
                r |= areaRect(d->hoveredArea);
            }
            d->hoveredArea = a;
            if (d->hoveredArea != NoArea) {
                r |= areaRect(d->hoveredArea);
                qApp->setOverrideCursor(cursorForArea(d->hoveredArea));
                d->cursorSet = true;
            }
            if (!r.isEmpty())
                update(r);
        }
    }

    Area activeArea() const { return d->activeArea; }
    void setActiveArea(Area a)
    {
        if (d->activeArea != a) {
            Rect r;
            if (d->cursorSet) {
                qApp->restoreOverrideCursor();
                d->cursorSet = false;
            }

            if (d->activeArea != NoArea) {
                r |= areaRect(d->activeArea);
            }

            d->activeArea = a;

            if (d->activeArea != NoArea) {
                r |= areaRect(d->hoveredArea);
            }
            if (!r.isEmpty())
                update(r);
        }
    }

    bool isEnabled() const { return d->enabled; }
    void setEnabled(bool e)
    {
        d->enabled = e;
        reset();
    }

    bool isPaintingEnabled() const { return d->enablePainting; }
    void setPaintingEnabled(bool e)
    {
        d->enablePainting = e;
        update();
        reset();
    }

    void reset()
    {
        if (d->cursorSet) {
            qApp->restoreOverrideCursor();
            d->cursorSet = false;
        }
        d->hoveredArea = d->activeArea = NoArea;
        update();
    }
private:
    struct Private {
        uint areas;
        Area hoveredArea, activeArea;
        bool enabled, cursorSet, enablePainting;
    } *d;
};

// class FloatResizer : public AbstractResizer<QPointF, QSizeF, QRectF>
// {
// public:
//     FloatResizer() {}
// };

class WidgetResizer : public QObject, public AbstractResizer<QPoint, QSize, QRect>
{
public:
    WidgetResizer(QWidget *w)
        : d(new Private) // not passing the parent
    {
        d->widget = 0;
        d->ignoreEvent = 0;
        d->hadMouseTracking = false;
        setWidget(w);
    }

    ~WidgetResizer()
    {
        delete d;
    }

    QWidget *widget() const
    {
        return d->widget;
    }

    void setWidget(QWidget * ww)
    {
        if (d->widget) {
            d->widget->removeEventFilter(this);
            if (!d->hadMouseTracking) {
                d->widget->setAttribute(Qt::WA_MouseTracking, false);
            }
        }
        reset();
        d->widget = ww;
        if (d->widget) {
            d->hadMouseTracking = d->widget->testAttribute(Qt::WA_MouseTracking);
            if (!d->hadMouseTracking) {
                d->widget->setAttribute(Qt::WA_MouseTracking, true);
            }
            d->widget->installEventFilter(this);
            update();
        }
    }

    bool eventFilter(QObject *o, QEvent *e)
    {
        if (o == d->widget && d->widget && e != d->ignoreEvent) {
            const QMouseEvent *me = static_cast<QMouseEvent*>(e);
            switch (e->type()) {
            case QEvent::Paint:
                if (isPaintingEnabled()) {
                    d->ignoreEvent = e;
                    qApp->sendEvent(o, e);
                    {
                        QPaintEvent *pe = static_cast<QPaintEvent*>(e);
                        QPainter p(d->widget);
                        paint(&p, pe->rect());
                    }
                    d->ignoreEvent = 0;
                    return true;
                }
                break;
            case QEvent::MouseButtonPress:
                return handleMousePress(me->pos() + d->widget->pos(), me->button());
            case QEvent::MouseButtonRelease:
                return handleMouseRelease(me->pos() + d->widget->pos(), me->button());
            case QEvent::MouseMove:
                return handleMouseMove(me->pos() + d->widget->pos(), me->buttons());
            case QEvent::Leave:
                reset();
                break;
            default:
                break;
            }
        }
        return false;
    }

    virtual void update(const QRect &rect = QRect())
    {
        if (d->widget) {
            if (rect.isEmpty()) {
                d->widget->update();
            } else {
                d->widget->update(rect.translated(-d->widget->pos()));
            }
        }
    }

    virtual void setGeometry(const QRect &r)
    {
        if (d->widget) {
            d->widget->setGeometry(r);
        }
    }

    virtual QRect geometry() const
    {
        if (d->widget) {
            return d->widget->geometry();
        }
        return QRect();
    }

protected:
    struct Private {
        QWidget *widget;
        QEvent *ignoreEvent;
        bool hadMouseTracking;
    } *d;
};

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
bool AbstractResizer<Point, Size, Rect>::handleMousePress(const Point &pos, Qt::MouseButton button)
{
    if (d->enabled && button == Qt::LeftButton) {
        setActiveArea(areaAt(pos));
        if (activeArea() != NoArea && !d->cursorSet) {
            qApp->setOverrideCursor(cursorForArea(activeArea()));
            d->cursorSet = true;
            return true;
        }
    }
    return false;
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
bool AbstractResizer<Point, Size, Rect>::handleMouseMove(const Point &pos, Qt::MouseButtons buttons)
{
    if (!d->enabled)
        return false;
    if (d->activeArea != NoArea) {
        Rect r = geometry();
        const Rect old = r;
        switch (d->activeArea) {
        case TopLeft: r.setTopLeft(pos); break;
        case Left: r.setLeft(pos.x()); break;
        case BottomLeft: r.setBottomLeft(pos); break;
        case Bottom: r.setBottom(pos.y()); break;
        case BottomRight: r.setBottomRight(pos); break;
        case Right: r.setRight(pos.x()); break;
        case TopRight: r.setTopRight(pos); break;
        case Top: r.setTop(pos.y()); break;
        case Center: r.moveCenter(pos); break;
        case NoArea:
        case AllAreas:
            Q_ASSERT(0);
            break;
        }
        if (r != old) {
            setGeometry(r);
        }
    } else if (buttons == Qt::NoButton) {
        setHoveredArea(areaAt(pos));
    } else {
        return false;
    }
    return true;
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
bool AbstractResizer<Point, Size, Rect>::handleMouseRelease(const Point &pos, Qt::MouseButton button)
{
    if (!d->enabled || d->activeArea == NoArea)
        return false;
    Q_UNUSED(pos);
    Q_UNUSED(button);
    setActiveArea(NoArea);
    return true;
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
Rect AbstractResizer<Point, Size, Rect>::areaRect(Area a) const
{
    if (!(d->areas & a)) {
        return Rect();
    }
    const Rect r = geometry();
    if (r.isEmpty())
        return Rect();
    Rect ret(r.topLeft(), areaSize(a));
    switch (a) {
    case NoArea:
    case AllAreas:
        return Rect(); // ### ASSERT?
    case TopLeft:
    case Left:
    case BottomLeft:
        break; // already at 0
    case Top:
    case Center:
    case Bottom:
        ret.moveLeft(r.center().x() - (ret.width() / 2));
        break;
    case TopRight:
    case Right:
    case BottomRight:
        ret.moveRight(r.right());
        break;
    }

    switch (a) {
    case NoArea:
    case AllAreas:
        return Rect(); // ### ASSERT?
    case TopLeft:
    case Top:
    case TopRight:
        break; // already at 0
    case Left:
    case Center:
    case Right:
        ret.moveTop(r.center().y() - (ret.height() / 2));
        break;
    case BottomLeft:
    case Bottom:
    case BottomRight:
        ret.moveBottom(r.bottom());
        break;
    }
    return ret;
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
QCursor AbstractResizer<Point, Size, Rect>::cursorForArea(Area a) const
{
    Qt::CursorShape shape = Qt::ArrowCursor;
    switch (a) {
    case TopLeft:
    case BottomRight: shape = Qt::SizeFDiagCursor; break;
    case Left:
    case Right: shape = Qt::SizeHorCursor; break;
    case BottomLeft:
    case TopRight: shape = Qt::SizeBDiagCursor; break;
    case Bottom:
    case Top: shape = Qt::SizeVerCursor; break;
    case Center: shape = Qt::SizeAllCursor; break;
    case AllAreas:
    case NoArea: return QCursor();
    }
    return QCursor(shape);
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
Size AbstractResizer<Point, Size, Rect>::areaSize(Area) const
{
    return Size(10, 10);
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
void AbstractResizer<Point, Size, Rect>::paint(QPainter *p, const Rect &c)
{
    if (!d->enabled)
        return;
    const Point offset = geometry().topLeft();
    const Rect clip = c.isEmpty() ? c : c.translated(offset);
    p->save();
    p->translate(-offset);
    Area area = TopLeft;
    while (area <= Center) {
        if (d->areas & area) {
            ResizerStyleOption opt;
            const Rect r = areaRect(area);
            if (clip.isEmpty() || clip.intersects(r)) {
                opt.area = area;
                opt.rect = r;
                if (d->activeArea == area) {
                    opt.state |= ResizerStyleOption::Active;
                }
                if (d->hoveredArea == area)
                    opt.state |= ResizerStyleOption::Hovered;
                paintArea(p, &opt);
            }
        }
        area = Area(area << 1);
    }
    p->restore();
}

template <class Point, class Size, class Rect> Q_INLINE_TEMPLATE
void AbstractResizer<Point, Size, Rect>::paintArea(QPainter *p, const ResizerStyleOption *opt)
{
    p->save();
    p->setPen(Qt::black);
    if (opt->state & ResizerStyleOption::Active) {
        p->setBrush(Qt::blue);
    } else if (opt->state & ResizerStyleOption::Hovered) {
        p->setBrush(Qt::green);
    } else {
        p->setBrush(Qt::yellow);
    }
    p->drawRect(opt->rect.adjusted(0, 0, -1, -1));
    p->restore();
}

#endif
