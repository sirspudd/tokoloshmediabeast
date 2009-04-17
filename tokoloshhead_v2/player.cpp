#include "player.h"
#include "widgets.h"
#include "config.h"
#include "tokolosh_interface.h"

// could use stringrefs as well

static inline const char *activateMember(QAbstractButton *)
{
    return "1animateClick()";
}

static inline const char *activateMember(QAction *)
{
    return "1trigger()";
}

static inline QString unquote(QString string)
{
    static QRegExp rx("^'(.*[^\\])'$");
    if (rx.exactMatch(string))
        string = rx.cap(1);

    string.replace("\\'", "'");
    return string;
}

template <class T>
static void setShortCuts(T *t, const char *member, const QKeySequence &defaultShortcut)
{
    qDeleteAll(qFindChildren<QShortcut*>(t));
    Q_ASSERT(!t->objectName().isEmpty());
    const QString value = Config::value<QString>(QString("Shortcuts/%1").arg(t->objectName()).simplified());
    if (value.isEmpty()) {
        t->setShortcut(defaultShortcut);
        return;
    }

    const QStringList list = value.split(' ', QString::SkipEmptyParts);
    for (int i=0; i<list.size(); ++i) {
        const QString str = ::unquote(list.at(i));
        QKeySequence key(str);
        if (key.isEmpty()) {
            qWarning("Can't decode key '%s'", qPrintable(str));
        }
        if (i == 0) {
            t->setShortcut(key);
        } else {
            new QShortcut(key, t, ::activateMember(t));
        }
    }


} // use this for both buttons and actions

Player::Player(QWidget *parent)
    : QWidget(parent)
{
    setContextMenuPolicy(Qt::ActionsContextMenu);
    d.channelMode = Private::Stereo;
    setFixedSize(275, 116);
    d.tokolosh = new TokoloshInterface("com.TokoloshXineBackend.TokoloshMediaPlayer",
                                       "/TokoloshMediaPlayer",
                                       QDBusConnection::sessionBus(),
                                       this);
    if (!Config::isEnabled("titlebar", false))
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocus();
    const char *separator = "";
    struct {
        const char *name;
        QObject *receiver;
        const char *member;
        const QString tooltip;
        const QRect rect;
        const bool checkable;
        const QKeySequence shortcut;
    } const buttonInfo[] = {
        // prev, pause, pause, stop, next
        { "Previous", d.tokolosh, SLOT(prev()), tr("Previous"),
          QRect(16, 86, 22, 18), false, Qt::Key_Z },
        { "Play", d.tokolosh, SLOT(play()), tr("Play"),
          QRect(38, 86, 22, 18), false, Qt::Key_X },
        { "Pause", d.tokolosh, SLOT(pause()), tr("Pause"),
          QRect(60, 86, 22, 18), false, Qt::Key_C },
        { "Stop", d.tokolosh, SLOT(stop()), tr("Stop"),
          QRect(82, 86, 22, 18), false, Qt::Key_V },
        { "Next", d.tokolosh, SLOT(next()), tr("Next"),
          QRect(104, 86, 22, 18), false, Qt::Key_B },
        { 0, 0, separator, QString(), QRect(), false, QKeySequence() },
        { "Open", this, SLOT(open()), tr("Open"), QRect(136, 87, 22, 16),
          false, QKeySequence::Open },
        { "OpenSkin", this, SLOT(openSkin()), tr("OpenSkin"),
          QRect(246, 84, 30, 20), false, Qt::ControlModifier + Qt::Key_S },
        { 0, 0, separator, QString(), QRect(), false, QKeySequence() },
        { "Shuffle", d.tokolosh, SLOT(toggleShuffle()), tr("Shuffle"),
          QRect(164, 86, 32, 15), true, Qt::ControlModifier + Qt::Key_Z }, // is there a shortcut for this one?
        { "Repeat", d.tokolosh, SLOT(repeat()), tr("Repeat"),
          QRect(209, 86, 32, 15), true, Qt::Key_R }, // is there a shortcut for this one?
        { 0, 0, separator, QString(), QRect(), false, QKeySequence() },
        { "Equalizer", d.tokolosh, SLOT(equalizer()), tr("Equalizer"),
          QRect(218, 57, 23, 13), true, Qt::Key_E },
        { "Playlist", d.tokolosh, SLOT(playlist()), tr("Playlist"),
          QRect(242, 57, 23, 13), true, Qt::Key_P },
        { 0, 0, separator, QString(), QRect(), false, QKeySequence() },
        { 0, 0, 0, QString(), QRect(), false, QKeySequence() }
    };

    int index = 0;
    for (int i=0; buttonInfo[i].member; ++i) {
        if (buttonInfo[i].member == separator) {
            QAction *sep = new QAction(this);
            sep->setSeparator(true);
            addAction(sep);
            continue;
        }
        Q_ASSERT(index < ButtonCount);
        d.buttons[index] = new Button(this);
        d.buttons[index]->setGeometry(buttonInfo[i].rect);
        d.buttons[index]->setObjectName(QString::fromLatin1(buttonInfo[i].name));
        d.buttons[index]->setToolTip(buttonInfo[i].tooltip);
        d.buttons[index]->setCheckable(buttonInfo[i].checkable);
        bool found = false;
        int shortcutIdx = 1;
        const QString key = QString("Shortcuts/%1%2").arg(d.buttons[index]->objectName());
        forever {
            const QString name = Config::value<QString>(key.arg(shortcutIdx == 1
                                                                ? QString()
                                                                : QString("_%1").arg(shortcutIdx))).
                                 trimmed();
            if (name.isEmpty())
                break;
            const QKeySequence shortcut(name);
            if (shortcut.isEmpty()) {
                break;
            }
            found = true;
            if (shortcutIdx > 1) {
                new QShortcut(shortcut, d.buttons[i], SLOT(animateClick()));
            } else {
                d.buttons[index]->setShortcut(shortcut);
            }
            ++shortcutIdx;
        }
        if (!found)
            d.buttons[index]->setShortcut(buttonInfo[i].shortcut);

        QAction *action = new QAction(buttonInfo[i].name, this);
        // ### icons?
        action->setCheckable(buttonInfo[i].checkable);
        connect(action, SIGNAL(triggered(bool)), buttonInfo[i].receiver, buttonInfo[i].member);
        connect(d.buttons[index], SIGNAL(clicked()), action, SLOT(trigger()));
        addAction(action);
        ++index;
    }

    struct {
        const QString name;
        const char *member;
        const QKeySequence shortcut;
    } const actions[] = {
        { tr("&Quit"), SLOT(close()), QKeySequence::Close },
// #ifdef QT_DEBUG
//         { tr("&Toggle debug geometry"), SLOT(toggleDebugGeometry()), },
// #endif
        { QString(), 0, QKeySequence() }
    };
    for (int i=0; actions[i].member; ++i) {
        QAction *action = new QAction(actions[i].name, this);
        if (actions[i].member == separator) {
            action->setSeparator(true);
        } else {
            action->setShortcut(actions[i].shortcut);
            connect(action, SIGNAL(triggered(bool)), this, actions[i].member);
        }
        addAction(action);
    }


    d.posBarSlider = new Slider(Qt::Horizontal, this);
    d.posBarSlider->setGeometry(14, 72, 253, 10);
    d.posBarSlider->setRange(0, 600);
    d.posBarSlider->setStyle(d.posBarStyle = new SliderStyle);

//     d.volumeSlider = new Slider(Qt::Horizontal, this);
//     d.volumeSlider->setGeometry(14, 72, 253, 10);
//     d.volumeSlider->setRange(0, 600);
//     d.volumeSlider->setStyle(d.volumeStyle = new SliderStyle);
    d.volumeSlider = 0;
}

void Player::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    d.main.render(&p);
//     d.numbers.render(&p, QPoint(10, 10), "143245");
//     d.text.render(&p, QPoint(150, 25), "THIS IS COOL");
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

static QPixmap findPixmap(const QString &dir, const QStringList &list, const char *buttonName,
                          bool warn = true)
{
    static const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QRegExp rx(QString("%1\\.(.*)$").arg(buttonName));
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    const int index = list.indexOf(rx);
    if (index != -1 && formats.contains(rx.cap(1).toLower().toLocal8Bit())) { // toLatin1()?
        QPixmap pix(QString("%1/%2").arg(dir).arg(list.at(index)));
        if (pix.isNull() && warn) {
            qWarning("Can't load image '%s'", qPrintable(QString("%1/%2").arg(dir).arg(list.at(index))));
        }
        return pix;
    } else if (warn) {
        qWarning("Can't find image '%s'", qPrintable(QString("%1/%2.*").arg(dir).arg(buttonName)));
    }
    return QPixmap();
}

static bool verifySkin(const QString &path, bool warn)
{
    const QDir dir(path);
    if (!dir.exists()) {
        if (warn)
            qWarning("%s doesn't seem to exist", qPrintable(path));
        return false;
    }

    const QStringList files = dir.entryList();
    static const char *fileNames[] = {
        "cbuttons", "eqmain", "monoster", /*"nums_ex",*/ "pledit",
        "shufrep", "titlebar", "eq_ex", "main", "numbers",
        "playpaus", "posbar", "text", "volume", 0
    };
    for (int i=0; fileNames[i]; ++i) {
        if (::findPixmap(dir.absolutePath(), files, fileNames[i], warn).isNull()) {
            return false;
        }
    }
    return true;
}

bool Player::setSkin(const QString &path)
{
    if (!::verifySkin(path, true)) {
        return false;
    }

    QDir dir(path);
    Q_ASSERT(dir.exists());
    const QStringList files = dir.entryList();
    struct {
        const char *name;
        const QRect sourceRect;
        const QRect targetRect;
        RenderObject *renderObject;
    } const buttons[] = {
        { "main", QRect(), QRect(), &d.main },

        { "cbuttons", QRect(0, 0, 22, 18), QRect(), &d.buttons[Previous]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(0, 18, 22, 18), QRect(), &d.buttons[Previous]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(22, 0, 22, 18), QRect(), &d.buttons[Play]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(22, 18, 22, 18), QRect(), &d.buttons[Play]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(44, 0, 22, 18), QRect(), &d.buttons[Pause]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(44, 18, 22, 18), QRect(), &d.buttons[Pause]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(66, 0, 22, 18), QRect(), &d.buttons[Next]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(66, 18, 22, 18), QRect(), &d.buttons[Next]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(88, 0, 22, 18), QRect(), &d.buttons[Stop]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(88, 18, 22, 18), QRect(), &d.buttons[Stop]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(114, 0, 22, 16), QRect(), &d.buttons[Open]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(114, 16, 22, 16), QRect(), &d.buttons[Open]->pixmaps[Button::Pressed] },

        { "shufrep", QRect(0, 0, 32, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Normal] },
        { "shufrep", QRect(0, 15, 32, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Pressed] },
        { "shufrep", QRect(0, 30, 32, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Checked] },
        { "shufrep", QRect(0, 45, 32, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Checked|Button::Pressed] },

        { "shufrep", QRect(28, 0, 32, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Normal] },
        { "shufrep", QRect(28, 15, 32, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Pressed] },
        { "shufrep", QRect(28, 30, 32, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Checked] },
        { "shufrep", QRect(28, 45, 32, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Checked|Button::Pressed] },

        { "shufrep", QRect(0, 61, 23, 13), QRect(), &d.buttons[Equalizer]->pixmaps[Button::Normal] },
        { "shufrep", QRect(0, 73, 23, 13), QRect(), &d.buttons[Equalizer]->pixmaps[Button::Checked] },

        { "shufrep", QRect(23, 61, 23, 13), QRect(), &d.buttons[Playlist]->pixmaps[Button::Normal] },
        { "shufrep", QRect(23, 73, 23, 13), QRect(), &d.buttons[Playlist]->pixmaps[Button::Checked] },

        { 0, QRect(), QRect(), 0 }
    };
    QHash<const char *, QPixmap> pixmaps;
    for (int i=0; buttons[i].name; ++i) {
        QPixmap pixmap;
        if (!pixmaps.contains(buttons[i].name)) {
            pixmap = ::findPixmap(dir.absolutePath(), files, buttons[i].name, false);
            pixmaps[buttons[i].name] = pixmap;
        } else {
            pixmap = pixmaps.value(buttons[i].name);
        }
        Q_ASSERT(buttons[i].renderObject);
        buttons[i].renderObject->pixmap = pixmap;
        Q_ASSERT(!pixmap.isNull());
        buttons[i].renderObject->sourceRect = buttons[i].sourceRect;
        buttons[i].renderObject->targetRect = buttons[i].targetRect;
    }

    struct {
        const char *name;
        const char *letters;
        const QSize letterSize; // do I need offset
        TextObject *textObject;
    } const textObjects[] = {
        { "numbers", "0123456789", QSize(9, 13), &d.numbers },
//        { "nums_ex", "0123456789 -", QSize(9, 13), &d.numbers }, // doesn't seem to exist everywhere. Not in YummiYogurt
        // ### is this @ larger than other stuff?
        // could make the last one on each line larger or something?
        // maybe a nasty hack like if char == '@'
        // what is this .. character and the one after the dollar?
        { "text",
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ\"@\n"
          "0123456789._:()-'!_+\\/[]^&%,=$#\n"
          "...?. ",
          QSize(5, 9), &d.text },
        { 0, 0, QSize(), 0 }
    };

    for (int i=0; textObjects[i].name; ++i) {
        TextObject *textObject = textObjects[i].textObject;
        textObject->pixmap = ::findPixmap(dir.absolutePath(), files, textObjects[i].name);
        Q_ASSERT(!textObject->pixmap.isNull());
        QRect sourceRect(QPoint(0, 0), textObjects[i].letterSize);
        for (int j=0; textObjects[i].letters[j]; ++j) {
            if (textObjects[i].letters[j] == '\n') {
                sourceRect.moveTopLeft(QPoint(0, sourceRect.y() + sourceRect.height()));
            } else {
                textObject->sourceRects[QLatin1Char(textObjects[i].letters[j])] = sourceRect;
//                 qDebug() << "setting" << textObjects[i].letters[j]
//                          << "to" << sourceRect.size();
                sourceRect.moveTopLeft(QPoint(sourceRect.x() + sourceRect.width(), sourceRect.y()));
            }
        }
    }
    const QPixmap sliderPixmap = ::findPixmap(dir.absolutePath(), files, "posbar");
    Q_ASSERT(!sliderPixmap.isNull());
    d.posBarStyle->normal.pixmap = sliderPixmap;
    d.posBarStyle->normal.sourceRect = QRect(278, 0, 30, 10);
    d.posBarStyle->normal.targetRect = QRect(0, 0, 30, 10);
    d.posBarStyle->pressed.pixmap = sliderPixmap;
    d.posBarStyle->pressed.sourceRect = QRect(248, 0, 30, 10);
    d.posBarStyle->pressed.targetRect = QRect(0, 0, 30, 10);
    d.posBarStyle->groovePressed.pixmap = sliderPixmap;
    d.posBarStyle->groovePressed.sourceRect = QRect(0, 0, 248, 10);
    // ### a lot of this stuff should be in Player::Player()
    return true;
}

void Player::showEvent(QShowEvent *e)
{
    activateWindow();
    raise();
    QWidget::showEvent(e);
}

void Player::open()
{
    const QStringList list = QFileDialog::getOpenFileNames(this, tr("Open files"),
                                                           Config::value<QString>("lastDirectory", QDir::homePath()),
                                                           tr("Music files(*.mp3 *.ogg *.flac)"));
    if (list.isEmpty())
        return;
    Config::setValue("lastDirectory", QFileInfo(list.first()).absolutePath());
    foreach(QString path, list) {
        d.tokolosh->load(path);
    }

    // ### need to query these extensions

}

void Player::openSkin()
{
    QFileDialog fd(this);
    fd.setFileMode(QFileDialog::DirectoryOnly);
    fd.setDirectory(Config::value<QString>("lastSkinDirectory", QCoreApplication::applicationDirPath()));
    if (fd.exec()) {
        Config::setValue<QString>("lastSkinDirectory", fd.directory().absolutePath());
    }
}
