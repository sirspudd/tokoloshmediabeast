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
    void render(QPainter *p) const // ### pass in clip?
    {
        const QRect sr = sourceRect.isNull() ? pixmap.rect() : sourceRect;
        const QRect tr = targetRect.isNull() ? QRect(QPoint(), sr.size()) : targetRect;
        p->drawPixmap(tr, pixmap, sr);
    }

    void render(QPainter *p, const QRect &tr) const // ### pass in clip?
    {
        const QRect sr = sourceRect.isNull() ? pixmap.rect() : sourceRect;
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

class WidgetResizer;
class Slider;
class SliderStyle;
class Button;
#ifdef QT_DEBUG
class Overlay;
#endif
class Player : public QWidget
{
    Q_OBJECT
public:
    Player(TokoloshInterface* dbusInterface, QWidget *parent = 0);
    ~Player();
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    bool setSkin(const QString &path);
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
public slots:
    void open();
    void openSkin();
    void reloadSettings();
    void editShortcuts();
    void wakeUp();
#ifdef QT_DEBUG
    void debugButton();
    void toggleOverlay(bool on);
    void toggleDebugGeometry(bool on);
#endif
private:
    enum ButtonType {
        Previous = 0,
        Play,
        Pause,
        Stop,
        Next,
        Open,
        Shuffle,
        Repeat,
        Equalizer,
        Playlist,
        ButtonCount
//        OpenSkin // what is the deal with this one. Is there a button for it?
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
        TokoloshInterface *dbusInterface;
        TextObject numbers, numbersEx, text;
        QPoint dragOffset;
        Slider *posBarSlider;
        SliderStyle *posBarStyle;
        Slider *volumeSlider;
        SliderStyle *volumeStyle;
        WidgetResizer *resizer;
        bool moving;
#ifdef QT_DEBUG
        Overlay *overlay;
#endif
    } d;
};

#endif
