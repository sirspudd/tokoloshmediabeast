#include "player.h"
#include "widgets.h"
#include "config.h"
#include <QDebug>
#include "tokolosh_interface.h"

static inline const char *activateMember(QAbstractButton *)
{
    return "1animateClick()";
}

static inline const char *activateMember(QAction *)
{
    return "1trigger()";
}

static inline QWidget *widget(QAction *a)
{
    return a->parentWidget();
}

static inline QWidget *widget(QWidget *a)
{
    return a;
}

// could use stringrefs
static inline QString unquote(QString string)
{
    static QRegExp rx("^'(.*[^\\\\])'$");
    if (rx.exactMatch(string))
        string = rx.cap(1);

    string.replace("\\'", "'");
    return string;
}

inline uint qHash(const QKeySequence &seq) { return qHash(seq.toString()); }
template <class T> static void setShortcuts(T *t)
{
    const QVariant defaultShortcutVariant = t->property("defaultShortcut");
    if (defaultShortcutVariant.isNull())
        return;
    const QKeySequence defaultShortcut = qVariantValue<QKeySequence>(defaultShortcutVariant);
    if (defaultShortcut.isEmpty())
        return;

    qDeleteAll(qFindChildren<QShortcut*>(t));
    Q_ASSERT(!t->objectName().isEmpty());
    const QString value = Config::value<QString>(QString("Shortcuts/%1").arg(t->objectName()).simplified());
    if (value.isEmpty()) {
        t->setShortcut(defaultShortcut);
        return;
    }

    const QStringList list = value.split(' ', QString::SkipEmptyParts);
    int idx = 0;
    QSet<QKeySequence> used;
    for (int i=0; i<list.size(); ++i) {
        const QString str = ::unquote(list.at(i));
        QKeySequence key;
        if (str == QLatin1String("[default]")) {
            key = defaultShortcut;
        } else {
            key = QKeySequence(str);
        }
        if (key.isEmpty()) {
            qWarning("Can't decode key '%s'", qPrintable(str));
            continue;
        }
        if (used.contains(key))
            continue;
        used.insert(key);
        if (idx == 0) {
            t->setShortcut(key);
        } else {
            new QShortcut(key, ::widget(t), ::activateMember(t));
        }

        ++idx;
    }


} // use this for both buttons and actions

Player::Player(TokoloshInterface* dbusInterface, QWidget *parent)
    : QWidget(parent)
{
#ifdef QT_DEBUG
    d.overlay = 0;
#endif

    setContextMenuPolicy(Qt::ActionsContextMenu);
    d.channelMode = Private::Stereo;
    setFixedSize(275, 116);
    bool ok;
    const QPoint pos = Config::value<QPoint>("position", QPoint(), &ok);
    if (ok && QApplication::desktop()->availableGeometry(this).contains(QRect(pos, size()))) {
        move(pos);
    }

    d.dbusInterface = dbusInterface;

    if (!Config::isEnabled("titlebar", false)) {
        Qt::WindowFlags flags = windowFlags() | Qt::FramelessWindowHint;
        if (Config::isEnabled("bypassx11", false))
            flags |= Qt::X11BypassWindowManagerHint;
        setWindowFlags(flags);
    }
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocus();
    const char *separator = "";
    struct {
        const char *name;
        QObject *receiver;
        const char *member;
        const QRect rect;
        const bool checkable;
        const QKeySequence shortcut;
    } const buttonInfo[] = {
        // prev, pause, pause, stop, next
        { QT_TRANSLATE_NOOP("Player", "Previous"), d.dbusInterface, SLOT(prev()),
          QRect(16, 88, 23, 18), false, Qt::Key_Z },
        { QT_TRANSLATE_NOOP("Player", "Play"), d.dbusInterface, SLOT(play()),
          QRect(39, 88, 23, 18), false, Qt::Key_X },
        { QT_TRANSLATE_NOOP("Player", "Pause"), d.dbusInterface, SLOT(pause()),
          QRect(62, 88, 23, 18), false, Qt::Key_C },
        { QT_TRANSLATE_NOOP("Player", "Stop"), d.dbusInterface, SLOT(stop()),
          QRect(85, 88, 23, 18), false, Qt::Key_V },
        { QT_TRANSLATE_NOOP("Player", "Next"), d.dbusInterface, SLOT(next()),
          QRect(108, 88, 22, 18), false, Qt::Key_B },
        { 0, 0, separator, QRect(), false, QKeySequence() },
        { QT_TRANSLATE_NOOP("Player", "Open"), this, SLOT(open()),
          QRect(136, 89, 22, 16), false, QKeySequence::Open },
        { QT_TRANSLATE_NOOP("Player", "OpenSkin"), this, SLOT(openSkin()),
          QRect(246, 84, 30, 20), false, Qt::ControlModifier + Qt::Key_S },
        { 0, 0, separator, QRect(), false, QKeySequence() },
        { QT_TRANSLATE_NOOP("Player", "Shuffle"), d.dbusInterface, SLOT(toggleShuffle()),
          QRect(164, 86, 32, 15), true, Qt::ControlModifier + Qt::Key_Z }, // is there a shortcut for this one?
        { QT_TRANSLATE_NOOP("Player", "Repeat"), d.dbusInterface, SLOT(repeat()),
          QRect(209, 86, 32, 15), true, Qt::Key_R }, // is there a shortcut for this one?
        { 0, 0, separator, QRect(), false, QKeySequence() },
        { QT_TRANSLATE_NOOP("Player", "Equalizer"), d.dbusInterface, SLOT(equalizer()),
          QRect(218, 57, 23, 13), true, Qt::Key_E },
        { QT_TRANSLATE_NOOP("Player", "Playlist"), d.dbusInterface, SLOT(playlist()),
          QRect(242, 57, 23, 13), true, Qt::Key_P },
        { 0, 0, separator, QRect(), false, QKeySequence() },
        { 0, 0, 0, QRect(), false, QKeySequence() }
    };

    int index = 0;
#ifdef QT_DEBUG
    const bool debugButtons = Config::isEnabled("debugButtons");
#endif
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
        d.buttons[index]->setToolTip(QApplication::translate("Playlist", buttonInfo[i].name));
        d.buttons[index]->setCheckable(buttonInfo[i].checkable);
        d.buttons[index]->setProperty("defaultShortcut", buttonInfo[i].shortcut);
        QAction *action = new QAction(buttonInfo[i].name, this);
        connect(action, SIGNAL(toggled(bool)), d.buttons[index], SLOT(setChecked(bool)));
        // ### icons?
        action->setCheckable(buttonInfo[i].checkable);
        connect(action, SIGNAL(triggered(bool)), buttonInfo[i].receiver, buttonInfo[i].member);
        connect(d.buttons[index], SIGNAL(clicked()), action, SLOT(trigger()));
#ifdef QT_DEBUG
        if (debugButtons)
            connect(d.buttons[index], SIGNAL(clicked()), this, SLOT(debugButton()));
#endif
        addAction(action);
        ++index;
        if (buttonInfo[i].checkable && Config::isEnabled(buttonInfo[i].name)) {
            QTimer::singleShot(0, action, SLOT(trigger()));
        }
    }

    enum CheckStatus {
        None = 0x0,
        Checkable = 0x1,
        Checked = 0x2
    };

    struct {
        const char *name;
        const char *member;
        const uint checkstatus;
        const QKeySequence shortcut;
    } const actionInfo[] = {
        { QT_TRANSLATE_NOOP("Player", "Quit"), SLOT(close()), None, QKeySequence::Close },
#ifdef QT_DEBUG
        { QT_TRANSLATE_NOOP("Player", "Toggle debug geometry"), SLOT(toggleDebugGeometry(bool)),
          (Config::isEnabled("debuggeometry") ? Checked|Checkable : Checkable),
          QKeySequence(Qt::AltModifier + Qt::Key_D) },
        { QT_TRANSLATE_NOOP("Player", "Toggle overlay"), SLOT(toggleOverlay(bool)),
          Checkable, QKeySequence(Qt::AltModifier | Qt::Key_O) },
#endif
        { 0, 0, QKeySequence(), false }
    };
    for (int i=0; actionInfo[i].member; ++i) {
        const QKeySequence shortcut = actionInfo[i].shortcut;
        QAction *action = new QAction(QCoreApplication::translate("Player", actionInfo[i].name), this);
        action->setObjectName(QString::fromLatin1(actionInfo[i].name));
        action->setCheckable(actionInfo[i].checkstatus & Checkable);
        action->setChecked(actionInfo[i].checkstatus & Checked);
        if (actionInfo[i].member == separator) {
            action->setSeparator(true);
        } else {
            action->setProperty("defaultShortcut", shortcut);
            connect(action, SIGNAL(triggered(bool)), this, actionInfo[i].member);
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
    reloadSettings();
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

        { "cbuttons", QRect(0, 0, 23, 18), QRect(), &d.buttons[Previous]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(0, 18, 23, 18), QRect(), &d.buttons[Previous]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(23, 0, 23, 18), QRect(), &d.buttons[Play]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(23, 18, 23, 18), QRect(), &d.buttons[Play]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(46, 0, 23, 18), QRect(), &d.buttons[Pause]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(46, 18, 23, 18), QRect(), &d.buttons[Pause]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(69, 0, 23, 18), QRect(), &d.buttons[Stop]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(69, 18, 23, 18), QRect(), &d.buttons[Stop]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(92, 0, 22, 18), QRect(), &d.buttons[Next]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(92, 18, 22, 18), QRect(), &d.buttons[Next]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(115, 0, 23, 16), QRect(), &d.buttons[Open]->pixmaps[Button::Normal] },
        { "cbuttons", QRect(115, 16, 23, 16), QRect(), &d.buttons[Open]->pixmaps[Button::Pressed] },

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
        d.dbusInterface->load(path);
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

void Player::reloadSettings()
{
    foreach(QAction *a, actions()) {
        ::setShortcuts(a);
    }

    for (int i=0; i<ButtonCount; ++i) {
        ::setShortcuts(d.buttons[i]);
    }
}


void Player::closeEvent(QCloseEvent *e)
{
    Config::setValue("position", pos());
    QWidget::closeEvent(e);
}

#ifdef QT_DEBUG
void Player::debugButton()
{
    qDebug() << sender()->objectName();
}
void Player::toggleDebugGeometry(bool on)
{
    Config::setEnabled("debuggeometry", on);
    update();
    foreach(QWidget *w, qFindChildren<QWidget*>(this)) {
        w->update();
    }
}

class Overlay : public QWidget
{
public:
    Overlay(const QPixmap &pix, QWidget *parent)
        : QWidget(parent)
    {
        QPalette pal = palette();
        pal.setBrush(backgroundRole(), pix);
        setPalette(pal);
        setAutoFillBackground(true);
        Q_ASSERT(parent);
        parent->installEventFilter(this);
        setGeometry(parent->rect());
        setAttribute(Qt::WA_MouseTracking);
    }

    void paintEvent(QPaintEvent *)
    {
        QPainter p(this);
        p.setPen(Qt::white);
        const bool debug = Config::isEnabled("debuggeometry");
        foreach(Button *b, qFindChildren<Button*>(window())) {
            QRect r = QRect(b->mapTo(window(), QPoint(0, 0)), b->size());
            if (debug)
                p.drawRect(r.adjusted(0, 0, -1, -1));

            if (r.contains(QCursor::pos() - window()->pos())) {
                p.fillRect(r, QColor(255, 0, 0, 70));
            }
        }
    }
    void mouseMoveEvent(QMouseEvent *)
    {
        update();
    }

    bool eventFilter(QObject *, QEvent *e)
    {
        if (e->type() == QEvent::Resize) {
            setGeometry(parentWidget()->rect());
        }
        return false;
    }
};

void Player::toggleOverlay(bool on)
{
    if (!d.overlay)
        d.overlay = new Overlay(QPixmap("original.png"), this);
    d.overlay->setVisible(on);
    if (on)
        d.overlay->raise();
}

#endif
