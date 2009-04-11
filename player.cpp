#include "player.h"
#include "config.h"

Button::Button(QWidget *parent)
    : QAbstractButton(parent)
{
}

void Button::paintEvent(QPaintEvent *)
{
    // ### use paintEvent->rect() ?
    int i = isChecked() ? Checked : Normal;
    if (isDown() && !pixmaps[i | Pressed].isNull())
        i |= Pressed;
    if (!pixmaps[i].isNull()) {
        QPainter p(this);
        pixmaps[i].render(&p, QPoint(0, 0));
    }
}

Player::Player(QWidget *parent)
    : QWidget(parent)
{
    d.channelMode = Private::Stereo;
    setFixedSize(275, 116);
    d.tokolosh = 0;
    if (!Config::isEnabled("titlebar", false))
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocus();
    struct {
        const QString name;
        const char *member;
        const QString tooltip;
        const QRect rect;
        const bool checkable;
    } const buttonInfo[] = {
        // prev, pause, pause, stop, next
        { tr("Previous"), SLOT(previous()), tr("Previous"), QRect(16, 86, 22, 18), false },
        { tr("Play"), SLOT(play()), tr("Play"), QRect(38, 86, 22, 18), false },
        { tr("Pause"), SLOT(pause()), tr("Pause"), QRect(60, 86, 22, 18), false },
        { tr("Stop"), SLOT(stop()), tr("Stop"), QRect(82, 86, 22, 18), false },
        { tr("Next"), SLOT(next()), tr("Next"), QRect(104, 86, 22, 18), false },
        { tr("Open"), SLOT(open()), tr("Open"), QRect(136, 87, 22, 16), false },
        { tr("OpenSkin"), SLOT(openSkin()), tr("OpenSkin"), QRect(246, 84, 30, 20), false },
        { tr("Shuffle"), SLOT(shuffle()), tr("Shuffle"), QRect(164, 86, 43, 15), true },
        { tr("Repeat"), SLOT(repeat()), tr("Repeat"), QRect(209, 86, 43, 15), true },
        { tr("Equalizer"), SLOT(equalizer()), tr("Equalizer"), QRect(219, 60, 23, 13), true },
        { tr("Playlist"), SLOT(playlist()), tr("Playlist"), QRect(242, 60, 23, 13), true },
        { QString(), 0, QString(), QRect(), 0 }
    };

    for (int i=0; buttonInfo[i].member; ++i) {
        Q_ASSERT(i < ButtonCount);
        d.buttons[i] = new Button(this);
        d.buttons[i]->setGeometry(buttonInfo[i].rect);
        d.buttons[i]->setObjectName(buttonInfo[i].name);
        d.buttons[i]->setToolTip(buttonInfo[i].tooltip);
        d.buttons[i]->setCheckable(buttonInfo[i].checkable);
        // connect to tokoloshinterface stuff
    }
}

void Player::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::blue);
    p.drawPixmap(e->rect(), d.main, e->rect()); // main brush instead?
//     if (d.channelMode == Private::Stereo) {
//         p.drawPixmap(d.stereo,
//     } else {

//     }
}

void Player::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        d.dragOffset = e->pos();
    }
}

void Player::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton) {
        move(e->globalPos() - d.dragOffset);
    }
}

static QPixmap findPixmap(const QString &dir, const QStringList &list, const char *buttonName)
{
    static const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QRegExp rx(QString("%1\\.(.*)$").arg(buttonName));
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    const int index = list.indexOf(rx);
    if (index != -1 && formats.contains(rx.cap(1).toLower().toLocal8Bit())) { // toLatin1()?
        QPixmap pix(QString("%1/%2").arg(dir).arg(list.at(index)));
        if (pix.isNull()) {
            qWarning("Can't load image '%s'", qPrintable(QString("%1/%2").arg(dir).arg(list.at(index))));
        }
        return pix;
    } else {
        qWarning("Can't find image '%s'", qPrintable(QString("%1/%2.*").arg(dir).arg(buttonName)));
    }
    return QPixmap();
}

bool Player::setSkin(const QString &path)
{
    const QDir dir(path);
    if (!dir.exists()) {
        qWarning("%s doesn't seem to exist", qPrintable(path));
        return false;
    }

    const QStringList files = dir.entryList();
    if (files.isEmpty()) {
        qWarning("No files here %s", qPrintable(dir.absolutePath()));
        return false;
    }
    struct {
        const char *name;
        const QRect rect;
        SubPixmap *pixmap;
    } const buttons[] = {
        { "main", QRect(), &d.main, 0 },

        { "cbuttons", QRect(0, 0, 22, 18),
          &d.buttons[Previous]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(0, 18, 22, 18),
          &d.buttons[Previous]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(22, 0, 22, 18),
          &d.buttons[Play]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(22, 18, 22, 18),
          &d.buttons[Play]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(44, 0, 22, 18),
          &d.buttons[Pause]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(44, 18, 22, 18),
          &d.buttons[Pause]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(66, 0, 22, 18),
          &d.buttons[Next]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(66, 18, 22, 18),
          &d.buttons[Next]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(88, 0, 22, 18),
          &d.buttons[Stop]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(88, 18, 22, 18),
          &d.buttons[Stop]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(114, 0, 22, 16),
          &d.buttons[Open]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(114, 16, 22, 16),
          &d.buttons[Open]->pixmaps[Button::Pressed] },

        { "shufrep", QRect(0, 0, 43, 15),
          &d.buttons[Repeat]->pixmaps[Button::Normal] },
        { "shufrep", QRect(0, 15, 43, 15),
          &d.buttons[Repeat]->pixmaps[Button::Pressed] },
        { "shufrep", QRect(0, 30, 43, 15),
          &d.buttons[Repeat]->pixmaps[Button::Checked] },
        { "shufrep", QRect(0, 45, 43, 15),
          &d.buttons[Repeat]->pixmaps[Button::Checked|Button::Pressed] },

        { "shufrep", QRect(28, 0, 43, 15),
          &d.buttons[Shuffle]->pixmaps[Button::Normal] },
        { "shufrep", QRect(28, 15, 43, 15),
          &d.buttons[Shuffle]->pixmaps[Button::Pressed] },
        { "shufrep", QRect(28, 30, 43, 15),
          &d.buttons[Shuffle]->pixmaps[Button::Checked] },
        { "shufrep", QRect(28, 45, 43, 15),
          &d.buttons[Shuffle]->pixmaps[Button::Checked|Button::Pressed] },

        { "shufrep", QRect(0, 61, 23, 13),
          &d.buttons[Equalizer]->pixmaps[Button::Normal] },
        { "shufrep", QRect(0, 73, 23, 13),
          &d.buttons[Equalizer]->pixmaps[Button::Checked] },

        { "shufrep", QRect(23, 61, 23, 13),
          &d.buttons[Playlist]->pixmaps[Button::Normal] },
        { "shufrep", QRect(23, 73, 23, 13),
          &d.buttons[Playlist]->pixmaps[Button::Checked] },

        { 0, QRect(), 0 }
    };
    QHash<const char *, QPixmap> pixmaps;
    for (int i=0; buttons[i].name; ++i) {
        if (!pixmaps.contains(buttons[i].name)) {
            const QPixmap pixmap = ::findPixmap(dir.absolutePath(), files, buttons[i].name);
            if (pixmap.isNull()) {
                qWarning("Skin invalid. Can't find %s in %s", buttons[i].name, qPrintable(dir.absolutePath()));
                return false;
            }
            pixmaps[buttons[i].name] = pixmap;
        }
    }

    for (int i=0; buttons[i].name; ++i) {
        Q_ASSERT(buttons[i].pixmap);
        Q_ASSERT(pixmaps.contains(buttons[i].name));
        *buttons[i].pixmap = pixmaps.value(buttons[i].name);
        if (buttons[i].sourceRect)
            *buttons[i].sourceRect = buttons[i].rect;
    }
//     QString main = ::findPixmap(files, "main");
//     qDebug() << main;
//     exit(0);
    return true;
}

    void Player::showEvent(QShowEvent *e)
    {
        activateWindow();
        raise();
        QWidget::showEvent(e);
    }
