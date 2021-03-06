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

#include "player.h"
#include "playlist.h"
#include "model.h"
#include "log.h"
#include "widgets.h"
#include "config.h"
#include "shortcutdialog.h"
#include <QDebug>
#include "resizer.h"
#include "skinselectiondialog.h"

enum SkinSelectionMechanism
{
    DirectlySelectFolder,
    SkinFolderListView
};

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

Q_DECLARE_METATYPE(QList<QShortcut*>);
inline uint qHash(const QKeySequence &seq) { return qHash(seq.toString()); }
template <class T> static void setShortcuts(T *t)
{
    Q_ASSERT(!t->objectName().isEmpty());
    const QKeySequence defaultShortcut = qVariantValue<QKeySequence>(t->property("defaultShortcut"));
    qDeleteAll(qFindChildren<QShortcut*>(t));
    const QString value = Config::value<QString>(QString("Shortcuts/%1").arg(t->objectName()).simplified());
    QSet<QKeySequence> shortcuts;
    if (!value.isEmpty()) {
        const QStringList list = value.split(' ', QString::SkipEmptyParts);
        for (int i=0; i<list.size(); ++i) {
            const QString str = ::unquote(list.at(i));
            QKeySequence key;
            if (str == "[default]") {
                key = defaultShortcut;
                if (defaultShortcut.isEmpty()) {
                    Log::log(0) << t->objectName() << "doesn't have a default shortcut";
                    continue;
                }
            } else {
                key = QKeySequence(str);
                if (key.isEmpty()) {
                    Log::log(0) << "Can't decode key" << str << "for" << t->objectName();
                    continue;
                }
            }
            shortcuts.insert(key);
        }
    } else if (!defaultShortcut.isEmpty()) {
        shortcuts.insert(defaultShortcut);
    }

    foreach(const QKeySequence &key, shortcuts) {
        Q_ASSERT(!key.isEmpty());
        QShortcut *shortcut = new QShortcut(key, ::widget(t));
        QObject::connect(shortcut, SIGNAL(activated()), t, ::activateMember(t));
        QList<QShortcut*> shortcuts = qVariantValue<QList<QShortcut*> >(t->property("shortcuts"));
        t->setProperty("shortcuts", qVariantFromValue<QList<QShortcut*> >(shortcuts));
    }
} // use this for both buttons and actions

Player::Player(QDBusInterface *interface, QWidget *parent)
    : QWidget(parent)
{
    d.moving = false;
    if (Config::isEnabled("resizer", true)) {
        d.resizer = new WidgetResizer(this);
        d.resizer->setAreas(AbstractResizer<QPoint, QSize, QRect>::AllAreas & ~AbstractResizer<QPoint, QSize, QRect>::Center);
        d.resizer->setPaintingEnabled(Config::isEnabled("resizePainting", false));
    } else {
        d.resizer = 0;
    }
#ifdef QT_DEBUG
    d.overlay = 0;
#endif

    setContextMenuPolicy(Qt::ActionsContextMenu);
    d.channelMode = Private::Stereo;
    if (!Config::isEnabled("ignorelastgeometry", false)) {
        const QByteArray g = Config::value<QByteArray>("geometry");
        if (!g.isEmpty()) {
            restoreGeometry(g);
            if (!QApplication::desktop()->availableGeometry(this).intersects(geometry())) {
                QRect r(QPoint(), sizeHint());
                r.moveCenter(QApplication::desktop()->availableGeometry(this).center());
                setGeometry(r);
            }
        }
    } else {
        QRect r(QPoint(), sizeHint());
        r.moveCenter(QApplication::desktop()->availableGeometry(this).center());
        setGeometry(r);
    }

    d.interface = interface;
    if (d.interface) {
        d.model = new TrackModel(d.interface, this);
        d.playlist = new PlaylistWidget(d.interface, d.model);
        d.interface->connection().connect(SERVICE_NAME, "/", QString(), "wakeUp", this, SLOT(wakeUp()));
    }

    if (!Config::isEnabled("titlebar", false)) {
        Qt::WindowFlags flags = windowFlags() | Qt::FramelessWindowHint;
        if (Config::isEnabled("bypassx11", false))
            flags |= Qt::X11BypassWindowManagerHint;
        setWindowFlags(flags);
    }
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocus();
//    const char *separator = "";
    struct {
        const char *name;
        QObject *receiver;
        const char *member;
        const QRect rect;
        const bool checkable;
        const QKeySequence shortcut;
    } const buttonInfo[] = {
        // prev, pause, pause, stop, next
        { QT_TRANSLATE_NOOP("Player", "Previous"), d.interface, SLOT(prev()),
          QRect(16, 88, 23, 18), false, Qt::Key_Z },
        { QT_TRANSLATE_NOOP("Player", "Play"), d.interface, SLOT(play()),
          QRect(39, 88, 23, 18), false, Qt::Key_X },
        { QT_TRANSLATE_NOOP("Player", "Pause"), d.interface, SLOT(pause()),
          QRect(62, 88, 23, 18), false, Qt::Key_C },
        { QT_TRANSLATE_NOOP("Player", "Stop"), d.interface, SLOT(stop()),
          QRect(85, 88, 23, 18), false, Qt::Key_V },
        { QT_TRANSLATE_NOOP("Player", "Next"), d.interface, SLOT(next()),
          QRect(108, 88, 22, 18), false, Qt::Key_B },
        { QT_TRANSLATE_NOOP("Player", "Open"), this, SLOT(open()),
          QRect(136, 89, 22, 16), false, QKeySequence::Open },
        { QT_TRANSLATE_NOOP("Player", "Shuffle"), d.interface, SLOT(setShuffle(bool)),
          QRect(164, 89, 46, 15), true, Qt::ControlModifier + Qt::Key_Z }, // is there a shortcut for this one?
        { QT_TRANSLATE_NOOP("Player", "Repeat"), d.interface, SLOT(setRepeat(bool)),
          QRect(210, 89, 28, 15), true, Qt::Key_R }, // is there a shortcut for this one?
        { QT_TRANSLATE_NOOP("Player", "Equalizer"), this, SLOT(setEqualizerVisible(bool)),
          QRect(219, 58, 23, 12), true, Qt::Key_E },
        { QT_TRANSLATE_NOOP("Player", "Playlist"), d.playlist, SLOT(setVisible(bool)),
          QRect(242, 58, 23, 12), true, Qt::ControlModifier + Qt::Key_P },
        { 0, 0, 0, QRect(), false, QKeySequence() },
    };

#ifdef QT_DEBUG
    const bool debugButtons = Config::isEnabled("debugButtons");
#endif
    for (int i=0; buttonInfo[i].member; ++i) {
        Q_ASSERT(i < ButtonCount);
        d.buttons[i] = new Button(this);
        d.buttons[i]->setObjectName(QString::fromLatin1(buttonInfo[i].name));
        const QString translated = QApplication::translate("Playlist", buttonInfo[i].name);
        d.buttons[i]->setToolTip(translated);
        d.buttons[i]->setText(translated); // used by shortcut editor
        d.buttons[i]->setProperty("defaultGeometry", buttonInfo[i].rect);
        d.buttons[i]->setProperty("defaultShortcut", buttonInfo[i].shortcut);
        d.buttons[i]->setGeometry(buttonInfo[i].rect);
        d.buttons[i]->setCheckable(buttonInfo[i].checkable);
        connect(d.buttons[i], buttonInfo[i].checkable
                ? SIGNAL(toggled(bool))
                : SIGNAL(clicked()), buttonInfo[i].receiver, buttonInfo[i].member);
#ifdef QT_DEBUG
        if (debugButtons)
            connect(d.buttons[i], SIGNAL(clicked()), this, SLOT(debugButton()));
#endif
        // ### icons?
        if (buttonInfo[i].checkable && Config::isEnabled(buttonInfo[i].name)) {
            QTimer::singleShot(0, d.buttons[i], SLOT(toggle()));
        }
    }

    enum CheckStatus {
        None = 0x0,
        Checkable = 0x1,
        Checked = 0x2
    };

    const char *separator = "";
    struct {
        const char *name;
        const char *member;
        const uint checkstatus;
        const QKeySequence shortcut;
    } const actionInfo[] = {
        { QT_TRANSLATE_NOOP("Player", "Quit"), SLOT(close()), None, QKeySequence::Close },
        { QT_TRANSLATE_NOOP("Player", "Edit shortcuts"), SLOT(editShortcuts()), None, QKeySequence() },
        { QT_TRANSLATE_NOOP("Player", "Restore default size"), SLOT(restoreDefaultSize()), None, QKeySequence() },
        { QT_TRANSLATE_NOOP("Player", "OpenSkin"), SLOT(openSkin()), None, QKeySequence(Qt::ControlModifier + Qt::Key_S) },
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
    d.posBarSlider->setProperty("defaultGeometry", d.posBarSlider->geometry());
    d.posBarSlider->setRange(0, 600);
    d.posBarSlider->setStyle(d.posBarStyle = new SliderStyle(this));

//    mainwin_volume = create_hslider(&mainwin_wlist, mainwin_bg, mainwin_gc, 107, 57, 68, 13, 15, 422, 0, 422, 14, 11, 15, 0, 0, 51, mainwin_volume_framecb, mainwin_volume_motioncb, mainwin_volume_releasecb, SKIN_VOLUME);

    d.volumeSlider = new Slider(Qt::Horizontal, this);
    d.volumeSlider->setGeometry(107, 57, 68, 13);
    d.volumeSlider->setRange(0, 100);
//    d.volumeSlider->setStyle(d.volumeStyle = new SliderStyle(this));
    reloadSettings();
    if (!setSkin(Config::value<QString>("skin", QString(":/skins/dullSod")))) {
        const bool ret = setSkin(":/skins/dullSod");
        Q_ASSERT(ret);
        Q_UNUSED(ret);
    }
    connect(d.playlist, SIGNAL(visibilityChanged(bool)), d.buttons[Playlist], SLOT(setChecked(bool)));
}

Player::~Player()
{
    delete d.resizer;
    delete d.posBarStyle;
}

void Player::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setTransform(qVariantValue<QTransform>(property("transform")));
    d.main.render(&p);
//     d.numbers.render(&p, QPoint(10, 10), "143245");
//     d.text.render(&p, QPoint(150, 25), "THIS IS COOL");
}

void Player::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        d.dragOffset = e->pos();
        d.moving = true;
    }
}

void Player::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton && d.moving) {
        move(e->globalPos() - d.dragOffset);
    }
}

void Player::mouseReleaseEvent(QMouseEvent *)
{
    d.moving = false;
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

static inline bool verifySkin(const QString &path, bool warn)
{
    const QDir dir(path);
    if (!dir.exists()) {
        if (warn)
            qWarning("%s doesnt seem't seem to exist", qPrintable(path));
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

bool Player::verifySkin(const QString &path)
{
    return ::verifySkin(path, false);
}


bool Player::setSkin(const QString &path)
{
    if (!::verifySkin(path, true)) {
        qDebug() << "Passed skin path is not acceptable";
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

        { "cbuttons", QRect(0, 0, 23, 18), QRect(), &d.buttons[Previous]->pixmaps[Button::Normal] }, // Previous
        { "cbuttons", QRect(0, 18, 23, 18), QRect(), &d.buttons[Previous]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(23, 0, 23, 18), QRect(), &d.buttons[Play]->pixmaps[Button::Normal] }, // Play
        { "cbuttons", QRect(23, 18, 23, 18), QRect(), &d.buttons[Play]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(46, 0, 23, 18), QRect(), &d.buttons[Pause]->pixmaps[Button::Normal] }, // Pause
        { "cbuttons", QRect(46, 18, 23, 18), QRect(), &d.buttons[Pause]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(69, 0, 23, 18), QRect(), &d.buttons[Stop]->pixmaps[Button::Normal] }, // Stop
        { "cbuttons", QRect(69, 18, 23, 18), QRect(), &d.buttons[Stop]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(92, 0, 22, 18), QRect(), &d.buttons[Next]->pixmaps[Button::Normal] }, // Next
        { "cbuttons", QRect(92, 18, 22, 18), QRect(), &d.buttons[Next]->pixmaps[Button::Pressed] },

        { "cbuttons", QRect(114, 0, 22, 16), QRect(), &d.buttons[Open]->pixmaps[Button::Normal] }, // Open
        { "cbuttons", QRect(114, 16, 22, 16), QRect(), &d.buttons[Open]->pixmaps[Button::Pressed] },

        { "shufrep", QRect(28, 0, 46, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Normal] }, // Shuffle
        { "shufrep", QRect(28, 15, 46, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Pressed] },
        { "shufrep", QRect(28, 30, 46, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Checked] },
        { "shufrep", QRect(28, 45, 46, 15), QRect(), &d.buttons[Shuffle]->pixmaps[Button::Checked|Button::Pressed] },

        { "shufrep", QRect(0, 0, 28, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Normal] }, // Repeat
        { "shufrep", QRect(0, 15, 28, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Pressed] },
        { "shufrep", QRect(0, 30, 28, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Checked] },
        { "shufrep", QRect(0, 45, 28, 15), QRect(), &d.buttons[Repeat]->pixmaps[Button::Checked|Button::Pressed] },

        { "shufrep", QRect(0, 61, 23, 12), QRect(), &d.buttons[Equalizer]->pixmaps[Button::Normal] }, // EQ
        { "shufrep", QRect(0, 73, 23, 12), QRect(), &d.buttons[Equalizer]->pixmaps[Button::Checked] },

        { "shufrep", QRect(23, 61, 23, 12), QRect(), &d.buttons[Playlist]->pixmaps[Button::Normal] }, // Playlist
        { "shufrep", QRect(23, 73, 23, 12), QRect(), &d.buttons[Playlist]->pixmaps[Button::Checked] },

        { 0, QRect(), QRect(), 0 }
    };

    QHash<const char *, QPixmap> pixmaps;
    const bool dumpPixmaps = Config::isEnabled("dump-pixmaps");
    const bool noSubPixmaps = !Config::isEnabled("subpixmaps", true);
    for (int i=0; buttons[i].name; ++i) {
        QPixmap pixmap;
        if (!pixmaps.contains(buttons[i].name)) {
            pixmap = ::findPixmap(dir.absolutePath(), files, buttons[i].name, false);
            pixmaps[buttons[i].name] = pixmap;
        } else {
            pixmap = pixmaps.value(buttons[i].name);
        }
        Q_ASSERT(buttons[i].renderObject);
        Q_ASSERT(!pixmap.isNull());
        if (noSubPixmaps) {
            pixmap = pixmap.copy(buttons[i].sourceRect);
        } else {
            buttons[i].renderObject->sourceRect = buttons[i].sourceRect;
        }
        buttons[i].renderObject->pixmap = pixmap;

        buttons[i].renderObject->targetRect = buttons[i].targetRect;
        Q_ASSERT(buttons[i].targetRect.isNull() || buttons[i].targetRect.size() == buttons[i].sourceRect.size());
        if (dumpPixmaps) {
            pixmap.copy(buttons[i].sourceRect).save(QString("%1_%2.png").arg(i).arg(buttons[i].name), "PNG");
        }

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
    update();
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
    foreach(const QString &path, list) {
        d.interface->asyncCall("load", path);
        // ### do I warn if it can't load it?
    }

    // ### need to query these extensions

}

void Player::openSkin()
{
    switch(Config::value<int>("skinSelectionMechism", DirectlySelectFolder)) {
    case SkinFolderListView:
        SkinSelectionDialog::instance(this)->show();
        break;
    case DirectlySelectFolder:
    {
        QFileDialog fd(this);
        fd.setFileMode(QFileDialog::DirectoryOnly);
        fd.setDirectory(Config::value<QString>("skin", QCoreApplication::applicationDirPath()));
        if (fd.exec()) {
            Config::setValue<QString>("skin", fd.directory().absolutePath());
            setSkin(Config::value<QString>("skin"));
            update();
        }
    }
    break;
    }
}

void Player::reloadSettings()
{
    foreach(QAction *a, actions()) {
//        qDebug() << a->objectName() << a;
        if (!a->isSeparator())
            ::setShortcuts(a);
    }

    for (int i=0; i<ButtonCount; ++i) {
        ::setShortcuts(d.buttons[i]);
//        qDebug() << d.buttons[i]->objectName() << d.buttons[i];
    }
}


void Player::closeEvent(QCloseEvent *e)
{
    Config::setValue("geometry", saveGeometry());
    QWidget::closeEvent(e);
    QApplication::quit();
}

void Player::editShortcuts()
{
    ShortcutDialog dialog(this);
    if (dialog.exec())
        reloadSettings();
}

void Player::wakeUp()
{
    activateWindow();
    raise();
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

void Player::resizeEvent(QResizeEvent *e)
{
    static const QSize defaultSize(275, 116);
    double sw = 1;
    double sh = 1;
    if (width() != defaultSize.width()) {
        sw = double(width()) / double(defaultSize.width());
    }

    if (height() != defaultSize.height()) {
        sh = double(height()) / double(defaultSize.height());
    }

    QTransform transform;
    transform.scale(sw, sh);
    setProperty("transform", transform);

    foreach(QWidget *w, qFindChildren<QWidget*>(this)) {
        const QRect defaultGeometry = w->property("defaultGeometry").toRect();
        if (!defaultGeometry.isNull()) {
            w->setGeometry(transform.mapRect(defaultGeometry));
        }
    }
    QWidget::resizeEvent(e);
}

QSize Player::sizeHint() const
{
    return QSize(275, 116);
}

void Player::restoreDefaultSize()
{
    resize(sizeHint());
}

void Player::togglePlaylist()
{
    d.playlist->setVisible(!d.playlist->isVisible());
}
