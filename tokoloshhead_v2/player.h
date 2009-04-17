#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui>
class TokoloshInterface;

struct RenderObject
{
    RenderObject() {}
    RenderObject(const QRect &tr, const QPixmap &pix, const QRect &sr)
        : pixmap(pix), targetRect(tr), sourceRect(sr)
    {}
    void render(QPainter *p) // ### pass in clip?
    {
        const QRect sr = sourceRect.isNull() ? pixmap.rect() : sourceRect;
        const QRect tr = targetRect.isNull() ? QRect(QPoint(), sr.size()) : targetRect;
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
            qDebug() << string.at(i) << r;
            pp += QPoint(r.width(), 0);
        }
    }

    QPixmap pixmap;
    QHash<QChar, QRect> sourceRects;
};

class SliderStyle : public QWindowsStyle
{
public:
    SliderStyle();
    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                    const QWidget *widget = 0) const;
//     virtual QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
//                                  SubControl sc, const QWidget *widget = 0) const;
    virtual int pixelMetric(PixelMetric m, const QStyleOption *opt = 0, const QWidget *widget = 0) const;
    virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                          const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
private:
    friend class Player;
    mutable RenderObject normal, pressed;
};

class Slider : public QSlider
{
public:
    Slider(Qt::Orientation o, QWidget *parent) : QSlider(o, parent) {}
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e) { QSlider::mouseReleaseEvent(e); e->accept(); }
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
    void timerEvent(QTimerEvent *e);
public slots:
    void open();
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

    struct Private {
        RenderObject main;
        Button *buttons[ButtonCount];
        enum ChannelMode { Stereo, Mono } channelMode;
        RenderObject elements[ElementCount];
        TokoloshInterface *tokolosh;
        TextObject numbers, numbersEx, text;
        QPoint dragOffset;
        QSlider *posBarSlider;
        SliderStyle posBarStyle; // does this get deleted by someone else?
    } d;
};

#endif
